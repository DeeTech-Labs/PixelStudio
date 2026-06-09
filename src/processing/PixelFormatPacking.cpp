#include "processing/PixelFormatPacking.h"

#include <cstring>
#include <algorithm>
#include <cmath>
#include <limits>
#include <QHash>
#include <QPair>

namespace {

struct McPixel {
    quint8 r;
    quint8 g;
    quint8 b;
};

struct McBox {
    int r1 = 0;
    int r2 = 255;
    int g1 = 0;
    int g2 = 255;
    int b1 = 0;
    int b2 = 255;
    QVector<int> indices;
};

int boxVolume(const McBox &box)
{
    return (box.r2 - box.r1 + 1) * (box.g2 - box.g1 + 1) * (box.b2 - box.b1 + 1);
}

int boxLongestAxis(const McBox &box)
{
    const int dr = box.r2 - box.r1;
    const int dg = box.g2 - box.g1;
    const int db = box.b2 - box.b1;
    if (dr >= dg && dr >= db)
        return 0;
    if (dg >= db)
        return 1;
    return 2;
}

quint8 axisValue(const McPixel &px, int axis)
{
    if (axis == 0)
        return px.r;
    if (axis == 1)
        return px.g;
    return px.b;
}

void shrinkBox(McBox *box, const QVector<McPixel> &pixels)
{
    if (box->indices.isEmpty())
        return;
    box->r1 = box->g1 = box->b1 = 255;
    box->r2 = box->g2 = box->b2 = 0;
    for (int idx : box->indices) {
        const McPixel &px = pixels.at(idx);
        box->r1 = qMin(box->r1, int(px.r));
        box->r2 = qMax(box->r2, int(px.r));
        box->g1 = qMin(box->g1, int(px.g));
        box->g2 = qMax(box->g2, int(px.g));
        box->b1 = qMin(box->b1, int(px.b));
        box->b2 = qMax(box->b2, int(px.b));
    }
}

quint32 averageColor(const McBox &box, const QVector<McPixel> &pixels)
{
    qint64 r = 0;
    qint64 g = 0;
    qint64 b = 0;
    for (int idx : box.indices) {
        const McPixel &px = pixels.at(idx);
        r += px.r;
        g += px.g;
        b += px.b;
    }
    const int n = box.indices.size();
    if (n < 1)
        return 0;
    return quint32(0xFF000000
                   | (quint32(r / n) << 16)
                   | (quint32(g / n) << 8)
                   | quint32(b / n));
}

int nearestPaletteIndex(quint8 r, quint8 g, quint8 b, const QVector<quint32> &palette)
{
    int best = 0;
    int bestDist = std::numeric_limits<int>::max();
    for (int i = 0; i < palette.size(); ++i) {
        const quint32 c = palette.at(i);
        const int dr = int(r) - int((c >> 16) & 0xFF);
        const int dg = int(g) - int((c >> 8) & 0xFF);
        const int db = int(b) - int(c & 0xFF);
        const int dist = dr * dr + dg * dg + db * db;
        if (dist < bestDist) {
            bestDist = dist;
            best = i;
        }
    }
    return best;
}

} // namespace

float PixelFormatPacking::srgbChannelToLinear(float channel)
{
    channel = qBound(0.0f, channel, 1.0f);
    if (channel <= 0.04045f)
        return channel / 12.92f;
    return std::pow((channel + 0.055f) / 1.055f, 2.4f);
}

quint8 PixelFormatPacking::linearChannelToSrgb8(float linear)
{
    linear = qBound(0.0f, linear, 1.0f);
    float encoded = linear;
    if (linear <= 0.0031308f)
        encoded = linear * 12.92f;
    else
        encoded = 1.055f * std::pow(linear, 1.0f / 2.4f) - 0.055f;
    return quint8(qBound(0, int(encoded * 255.0f + 0.5f), 255));
}

quint8 PixelFormatPacking::quantizeLinearToGray8(float linear)
{
    return linearChannelToSrgb8(qBound(0.0f, linear, 1.0f));
}

quint16 PixelFormatPacking::quantizeSrgbToRgb565(quint8 r, quint8 g, quint8 b)
{
    const quint16 r5 = quint16((r >> 3) & 0x1F);
    const quint16 g6 = quint16((g >> 2) & 0x3F);
    const quint16 b5 = quint16((b >> 3) & 0x1F);
    return quint16((r5 << 11) | (g6 << 5) | b5);
}

quint16 PixelFormatPacking::quantizeLinearToRgb565(float rLin, float gLin, float bLin)
{
    const int r5 = qBound(0, int(rLin * 31.0f + 0.5f), 31);
    const int g6 = qBound(0, int(gLin * 63.0f + 0.5f), 63);
    const int b5 = qBound(0, int(bLin * 31.0f + 0.5f), 31);
    return quint16((quint16(r5) << 11) | (quint16(g6) << 5) | quint16(b5));
}

quint16 PixelFormatPacking::swapRgb565ToBgr565(quint16 rgb565)
{
    const quint8 r5 = quint8((rgb565 >> 11) & 0x1F);
    const quint8 g6 = quint8((rgb565 >> 5) & 0x3F);
    const quint8 b5 = quint8(rgb565 & 0x1F);
    return quint16((quint16(b5) << 11) | (quint16(g6) << 5) | r5);
}

QByteArray PixelFormatPacking::padForDma(QByteArray data, int alignBytes)
{
    if (alignBytes < 2)
        return data;
    while (data.size() % alignBytes != 0)
        data.append(char(0));
    return data;
}

QByteArray PixelFormatPacking::packUint16Le(quint16 value)
{
    QByteArray out(2, 0);
    out[0] = char(value & 0xFF);
    out[1] = char((value >> 8) & 0xFF);
    return out;
}

QByteArray PixelFormatPacking::packUint16Be(quint16 value)
{
    QByteArray out(2, 0);
    out[0] = char((value >> 8) & 0xFF);
    out[1] = char(value & 0xFF);
    return out;
}

QByteArray PixelFormatPacking::packRgb565Stream(const QVector<quint16> &rgb565, int pixels, bool bigEndian)
{
    QByteArray out;
    out.reserve(pixels * 2);
    for (int i = 0; i < pixels && i < rgb565.size(); ++i) {
        const quint16 v = rgb565.at(i);
        out.append(bigEndian ? packUint16Be(v) : packUint16Le(v));
    }
    return out;
}

QByteArray PixelFormatPacking::packRgb666Compact(const QByteArray &rgb888, int pixels)
{
    if (rgb888.size() != pixels * 3)
        return QByteArray((pixels * 18 + 7) / 8, 0);

    QByteArray out((pixels * 18 + 7) / 8, 0);
    quint64 buffer = 0;
    int bits = 0;
    int outIdx = 0;
    for (int i = 0; i < pixels; ++i) {
        const quint32 r6 = quint32(quint8(rgb888.at(i * 3)) >> 2) & 0x3Fu;
        const quint32 g6 = quint32(quint8(rgb888.at(i * 3 + 1)) >> 2) & 0x3Fu;
        const quint32 b6 = quint32(quint8(rgb888.at(i * 3 + 2)) >> 2) & 0x3Fu;
        const quint32 px18 = (r6 << 12) | (g6 << 6) | b6;
        buffer = (buffer << 18) | quint64(px18);
        bits += 18;
        while (bits >= 8 && outIdx < out.size()) {
            bits -= 8;
            out[outIdx++] = char((buffer >> bits) & 0xFF);
        }
    }
    if (bits > 0 && outIdx < out.size())
        out[outIdx] = char((buffer << (8 - bits)) & 0xFF);
    return out;
}

void PixelFormatPacking::decodeRgb666Compact(const QByteArray &packed, int pixels, QByteArray &rgb888Out)
{
    rgb888Out.resize(pixels * 3);
    quint64 buffer = 0;
    int bits = 0;
    int inIdx = 0;
    for (int i = 0; i < pixels; ++i) {
        while (bits < 18 && inIdx < packed.size()) {
            buffer = (buffer << 8) | quint64(quint8(packed.at(inIdx++)));
            bits += 8;
        }
        bits -= 18;
        const quint32 px18 = quint32((buffer >> bits) & 0x3FFFFu);
        const int r = int((px18 >> 12) & 0x3Fu) * 255 / 63;
        const int g = int((px18 >> 6) & 0x3Fu) * 255 / 63;
        const int b = int(px18 & 0x3Fu) * 255 / 63;
        rgb888Out[i * 3] = char(r);
        rgb888Out[i * 3 + 1] = char(g);
        rgb888Out[i * 3 + 2] = char(b);
    }
}

QByteArray PixelFormatPacking::packHalfLe(const QVector<quint16> &half)
{
    QByteArray out;
    out.reserve(half.size() * 2);
    for (quint16 v : half)
        out.append(packUint16Le(v));
    return out;
}

QByteArray PixelFormatPacking::packFloatLe(const QVector<float> &values)
{
    QByteArray out;
    out.reserve(values.size() * 4);
    for (float f : values) {
        quint32 bits = 0;
        static_assert(sizeof(float) == sizeof(quint32));
        std::memcpy(&bits, &f, sizeof(bits));
        out.append(char(bits & 0xFF));
        out.append(char((bits >> 8) & 0xFF));
        out.append(char((bits >> 16) & 0xFF));
        out.append(char((bits >> 24) & 0xFF));
    }
    return out;
}

QByteArray PixelFormatPacking::packMonoVerticalCol(const QVector<bool> &bits, int width, int height)
{
    const int pages = (height + 7) / 8;
    QByteArray out(width * pages, 0);
    for (int x = 0; x < width; ++x) {
        for (int page = 0; page < pages; ++page) {
            quint8 byte = 0;
            for (int bit = 0; bit < 8; ++bit) {
                const int y = page * 8 + bit;
                if (y < height && bits.at(y * width + x))
                    byte |= quint8(1u << bit);
            }
            out[x * pages + page] = char(byte);
        }
    }
    return out;
}

void PixelFormatPacking::buildIndexedMedianCut(const QByteArray &rgb888,
                                               int pixels,
                                               QByteArray &indicesOut,
                                               QVector<quint32> &paletteRgbOut)
{
    indicesOut = QByteArray(pixels, 0);
    paletteRgbOut.clear();
    if (rgb888.size() != pixels * 3 || pixels < 1)
        return;

    QVector<McPixel> pixelsData(pixels);
    for (int i = 0; i < pixels; ++i) {
        pixelsData[i].r = quint8(rgb888.at(i * 3));
        pixelsData[i].g = quint8(rgb888.at(i * 3 + 1));
        pixelsData[i].b = quint8(rgb888.at(i * 3 + 2));
    }

    QVector<McBox> boxes(1);
    boxes[0].indices.resize(pixels);
    for (int i = 0; i < pixels; ++i)
        boxes[0].indices[i] = i;
    shrinkBox(&boxes[0], pixelsData);

    while (boxes.size() < 256) {
        int splitIndex = -1;
        int bestVol = -1;
        for (int i = 0; i < boxes.size(); ++i) {
            if (boxes.at(i).indices.size() < 2)
                continue;
            const int vol = boxVolume(boxes.at(i));
            if (vol > bestVol) {
                bestVol = vol;
                splitIndex = i;
            }
        }
        if (splitIndex < 0)
            break;

        McBox box = boxes.takeAt(splitIndex);
        const int axis = boxLongestAxis(box);
        std::sort(box.indices.begin(), box.indices.end(), [&](int a, int b) {
            return axisValue(pixelsData.at(a), axis) < axisValue(pixelsData.at(b), axis);
        });
        const int mid = box.indices.size() / 2;
        McBox left;
        McBox right;
        left.indices = box.indices.mid(0, mid);
        right.indices = box.indices.mid(mid);
        shrinkBox(&left, pixelsData);
        shrinkBox(&right, pixelsData);
        boxes.append(left);
        boxes.append(right);
    }

    paletteRgbOut.reserve(boxes.size());
    for (const McBox &box : boxes)
        paletteRgbOut.append(averageColor(box, pixelsData));

    for (int i = 0; i < pixels; ++i) {
        const McPixel &px = pixelsData.at(i);
        indicesOut[i] = char(nearestPaletteIndex(px.r, px.g, px.b, paletteRgbOut));
    }
}

void PixelFormatPacking::rgbToYuv(quint8 r, quint8 g, quint8 b, quint8 *y, quint8 *u, quint8 *v)
{
    const int yi = (77 * int(r) + 150 * int(g) + 29 * int(b)) >> 8;
    const int ui = (-43 * int(r) - 85 * int(g) + 128 * int(b) + 32768) >> 8;
    const int vi = (128 * int(r) - 107 * int(g) - 21 * int(b) + 32768) >> 8;
    *y = quint8(qBound(0, yi, 255));
    *u = quint8(qBound(0, ui, 255));
    *v = quint8(qBound(0, vi, 255));
}

void PixelFormatPacking::yuvToRgb(quint8 y, quint8 u, quint8 v, int *r, int *g, int *b)
{
    const int uu = int(u) - 128;
    const int vv = int(v) - 128;
    *r = qBound(0, int(y) + ((1436 * vv) >> 10), 255);
    *g = qBound(0, int(y) - ((352 * uu + 731 * vv) >> 10), 255);
    *b = qBound(0, int(y) + ((1815 * uu) >> 10), 255);
}

namespace {

void averageChromaBlock(const QByteArray &rgb888,
                        int width,
                        int height,
                        int blockX,
                        int blockY,
                        quint8 *uOut,
                        quint8 *vOut)
{
    int uSum = 0;
    int vSum = 0;
    int count = 0;
    for (int dy = 0; dy < 2; ++dy) {
        const int py = blockY + dy;
        if (py >= height)
            continue;
        for (int dx = 0; dx < 2; ++dx) {
            const int px = blockX + dx;
            if (px >= width)
                continue;
            const int i = (py * width + px) * 3;
            quint8 yy, uu, vv;
            PixelFormatPacking::rgbToYuv(quint8(rgb888.at(i)),
                                         quint8(rgb888.at(i + 1)),
                                         quint8(rgb888.at(i + 2)),
                                         &yy,
                                         &uu,
                                         &vv);
            uSum += uu;
            vSum += vv;
            ++count;
        }
    }
    if (count < 1) {
        *uOut = 128;
        *vOut = 128;
        return;
    }
    *uOut = quint8(uSum / count);
    *vOut = quint8(vSum / count);
}

} // namespace

QByteArray PixelFormatPacking::packYuvYuyv(const QByteArray &rgb888, int width, int height)
{
    const int pixels = width * height;
    if (rgb888.size() != pixels * 3)
        return QByteArray(pixels * 2, 0);

    QByteArray out(pixels * 2, 0);
    int o = 0;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; x += 2) {
            const int i0 = (y * width + x) * 3;
            quint8 y0, u0, v0, y1;
            rgbToYuv(quint8(rgb888.at(i0)), quint8(rgb888.at(i0 + 1)), quint8(rgb888.at(i0 + 2)), &y0, &u0, &v0);
            if (x + 1 < width) {
                const int i1 = (y * width + x + 1) * 3;
                quint8 u1, v1;
                rgbToYuv(quint8(rgb888.at(i1)), quint8(rgb888.at(i1 + 1)), quint8(rgb888.at(i1 + 2)), &y1, &u1, &v1);
                out[o++] = char(y0);
                out[o++] = char(quint8((int(u0) + int(u1)) / 2));
                out[o++] = char(y1);
                out[o++] = char(quint8((int(v0) + int(v1)) / 2));
            } else {
                out[o++] = char(y0);
                out[o++] = char(u0);
                out[o++] = char(y0);
                out[o++] = char(v0);
            }
        }
    }
    return out;
}

QByteArray PixelFormatPacking::packYuvNv12(const QByteArray &rgb888, int width, int height)
{
    const int pixels = width * height;
    const int uvStride = (width + 1) & ~1;
    const int chromaRows = (height + 1) / 2;
    const int uvBytes = uvStride * chromaRows;
    if (rgb888.size() != pixels * 3)
        return QByteArray(pixels + uvBytes, 0);

    QByteArray out(pixels + uvBytes, 0);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const int i = (y * width + x) * 3;
            quint8 lum, u, v;
            rgbToYuv(quint8(rgb888.at(i)), quint8(rgb888.at(i + 1)), quint8(rgb888.at(i + 2)), &lum, &u, &v);
            out[y * width + x] = char(lum);
        }
    }

    for (int cy = 0; cy < chromaRows; ++cy) {
        for (int cx = 0; cx < uvStride; cx += 2) {
            quint8 u = 128;
            quint8 v = 128;
            averageChromaBlock(rgb888, width, height, cx, cy * 2, &u, &v);
            const int uvIndex = pixels + cy * uvStride + cx;
            out[uvIndex] = char(u);
            out[uvIndex + 1] = char(v);
        }
    }
    return out;
}

QByteArray PixelFormatPacking::packYuvYv12(const QByteArray &rgb888, int width, int height)
{
    const int pixels = width * height;
    const int chromaWidth = (width + 1) / 2;
    const int chromaRows = (height + 1) / 2;
    const int chromaPixels = chromaWidth * chromaRows;
    if (rgb888.size() != pixels * 3)
        return QByteArray(pixels + chromaPixels * 2, 0);

    QByteArray out(pixels + chromaPixels * 2, 0);
    for (int i = 0; i < pixels; ++i) {
        quint8 y, u, v;
        rgbToYuv(quint8(rgb888.at(i * 3)), quint8(rgb888.at(i * 3 + 1)), quint8(rgb888.at(i * 3 + 2)), &y, &u, &v);
        out[i] = char(y);
    }

    const int uOff = pixels;
    const int vOff = pixels + chromaPixels;
    for (int cy = 0; cy < chromaRows; ++cy) {
        for (int cx = 0; cx < chromaWidth; ++cx) {
            quint8 u = 128;
            quint8 v = 128;
            averageChromaBlock(rgb888, width, height, cx * 2, cy * 2, &u, &v);
            const int ci = cy * chromaWidth + cx;
            out[uOff + ci] = char(u);
            out[vOff + ci] = char(v);
        }
    }
    return out;
}
