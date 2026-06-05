#include "processing/DisplayRasterizer.h"
#include "processing/PixelFormatCatalog.h"
#include "processing/PixelFormatPacking.h"
#include <QPainter>
#include <QtMath>
#include <QHash>
#include <algorithm>
#include <limits>

namespace {

QImage scaleToTarget(const QImage &source, int tw, int th, DisplayProfile::ScaleMode mode)
{
    if (source.isNull() || tw < 1 || th < 1)
        return {};

    switch (mode) {
    case DisplayProfile::Stretch:
        return source.scaled(tw, th, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    case DisplayProfile::Crop: {
        const qreal srcAspect = qreal(source.width()) / qreal(source.height());
        const qreal dstAspect = qreal(tw) / qreal(th);
        int cropW = source.width();
        int cropH = source.height();
        if (srcAspect > dstAspect) {
            cropW = qRound(source.height() * dstAspect);
        } else {
            cropH = qRound(source.width() / dstAspect);
        }
        const int x = (source.width() - cropW) / 2;
        const int y = (source.height() - cropH) / 2;
        return source.copy(x, y, cropW, cropH)
            .scaled(tw, th, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }
    case DisplayProfile::Fit:
    default: {
        QImage canvas(tw, th, QImage::Format_ARGB32);
        canvas.fill(Qt::black);
        const QImage scaled = source.scaled(tw, th, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        const int x = (tw - scaled.width()) / 2;
        const int y = (th - scaled.height()) / 2;
        QPainter p(&canvas);
        p.drawImage(x, y, scaled);
        p.end();
        return canvas;
    }
    }
}

int grayValue(QRgb c)
{
    return qRound(0.299 * qRed(c) + 0.587 * qGreen(c) + 0.114 * qBlue(c));
}

quint16 toRgb565(QRgb c)
{
    const quint16 r = quint16((qRed(c) >> 3) & 0x1F);
    const quint16 g = quint16((qGreen(c) >> 2) & 0x3F);
    const quint16 b = quint16((qBlue(c) >> 3) & 0x1F);
    return quint16((r << 11) | (g << 5) | b);
}

quint8 toRgb233(QRgb c)
{
    const quint8 r = quint8((qRed(c) >> 5) & 0x07);
    const quint8 g = quint8((qGreen(c) >> 5) & 0x07);
    const quint8 b = quint8((qBlue(c) >> 6) & 0x03);
    return quint8((r << 5) | (g << 2) | b);
}

QVector<bool> monoBitsFloydSteinberg(const QImage &img, int threshold)
{
    const int w = img.width();
    const int h = img.height();
    QVector<qreal> err(w * h);
    QVector<bool> out(w * h, false);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            const int idx = y * w + x;
            const qreal old = qreal(grayValue(img.pixel(x, y))) + err[idx];
            const bool on = old >= threshold;
            out[idx] = on;
            const qreal diff = old - (on ? 255.0 : 0.0);
            if (x + 1 < w)
                err[idx + 1] += diff * 7.0 / 16.0;
            if (y + 1 < h) {
                if (x > 0)
                    err[idx + w - 1] += diff * 3.0 / 16.0;
                err[idx + w] += diff * 5.0 / 16.0;
                if (x + 1 < w)
                    err[idx + w + 1] += diff * 1.0 / 16.0;
            }
        }
    }
    return out;
}

QByteArray packSsd1306(const QVector<bool> &bits, int w, int h)
{
    const int pages = (h + 7) / 8;
    QByteArray buf(w * pages, 0);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            if (!bits[y * w + x])
                continue;
            const int page = y / 8;
            const int bit = y % 8;
            buf[page * w + x] |= char(1 << bit);
        }
    }
    return buf;
}

QRgb fromRgb565(quint16 v)
{
    const int r = ((v >> 11) & 0x1F) * 255 / 31;
    const int g = ((v >> 5) & 0x3F) * 255 / 63;
    const int b = (v & 0x1F) * 255 / 31;
    return qRgb(r, g, b);
}

quint16 toBgr565Word(quint16 rgb565)
{
    const quint8 r5 = quint8((rgb565 >> 11) & 0x1F);
    const quint8 g6 = quint8((rgb565 >> 5) & 0x3F);
    const quint8 b5 = quint8(rgb565 & 0x1F);
    return quint16((quint16(b5) << 11) | (quint16(g6) << 5) | r5);
}

QRgb fromBgr565(quint16 v)
{
    const int b = ((v >> 11) & 0x1F) * 255 / 31;
    const int g = ((v >> 5) & 0x3F) * 255 / 63;
    const int r = (v & 0x1F) * 255 / 31;
    return qRgb(r, g, b);
}

QImage previewFromRgb565(const QVector<quint16> &pixels, int w, int h)
{
    QImage img(w, h, QImage::Format_ARGB32);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            img.setPixel(x, y, fromRgb565(pixels[y * w + x]));
        }
    }
    return img;
}

QImage previewFromMono(const QVector<bool> &bits, int w, int h)
{
    QImage img(w, h, QImage::Format_ARGB32);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            const bool on = bits[y * w + x];
            img.setPixel(x, y, on ? qRgb(255, 255, 255) : qRgb(0, 0, 0));
        }
    }
    return img;
}

QImage previewFromGrayscale8(const QByteArray &gray, int w, int h)
{
    QImage img(w, h, QImage::Format_ARGB32);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            const int g = uchar(gray.at(y * w + x));
            img.setPixel(x, y, qRgb(g, g, g));
        }
    }
    return img;
}

QImage previewFromGrayscale4(const QByteArray &gray, int w, int h)
{
    QImage img(w, h, QImage::Format_ARGB32);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            const int nibble = (uchar(gray.at(y * w + x)) >> 4) & 0x0F;
            const int g = nibble * 255 / 15;
            img.setPixel(x, y, qRgb(g, g, g));
        }
    }
    return img;
}

QImage previewFromBgr565(const QVector<quint16> &pixels, int w, int h)
{
    QImage img(w, h, QImage::Format_ARGB32);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            img.setPixel(x, y, fromBgr565(toBgr565Word(pixels[y * w + x])));
        }
    }
    return img;
}

QImage previewFromBgr888(const QByteArray &packed, int w, int h)
{
    QImage img(w, h, QImage::Format_ARGB32);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            const int i = (y * w + x) * 3;
            img.setPixel(x, y, qRgb(uchar(packed.at(i + 2)),
                                    uchar(packed.at(i + 1)),
                                    uchar(packed.at(i))));
        }
    }
    return img;
}

QImage previewFromIndexed8(const QByteArray &indices,
                           const QVector<quint32> &palette,
                           int w,
                           int h)
{
    QImage img(w, h, QImage::Format_ARGB32);
    if (palette.isEmpty())
        return img;
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            const int idx = uchar(indices.at(y * w + x)) % palette.size();
            const quint32 c = palette.at(idx);
            img.setPixel(x, y, qRgb((c >> 16) & 0xFF, (c >> 8) & 0xFF, c & 0xFF));
        }
    }
    return img;
}


QImage previewFromYuvYuyv(const QByteArray &packed, int w, int h)
{
    QImage img(w, h, QImage::Format_ARGB32);
    int o = 0;
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; x += 2) {
            if (o + 3 >= packed.size())
                break;
            const quint8 y0 = quint8(packed.at(o++));
            const quint8 u = quint8(packed.at(o++));
            const quint8 y1 = quint8(packed.at(o++));
            const quint8 v = quint8(packed.at(o++));
            int r, g, b;
            PixelFormatPacking::yuvToRgb(y0, u, v, &r, &g, &b);
            img.setPixel(x, y, qRgb(r, g, b));
            if (x + 1 < w) {
                PixelFormatPacking::yuvToRgb(y1, u, v, &r, &g, &b);
                img.setPixel(x + 1, y, qRgb(r, g, b));
            }
        }
    }
    return img;
}

QImage previewFromYuvNv12(const QByteArray &packed, int w, int h)
{
    QImage img(w, h, QImage::Format_ARGB32);
    const int pixels = w * h;
    const int uvStride = (w + 1) & ~1;
    const int chromaRows = (h + 1) / 2;
    if (packed.size() < pixels + uvStride * chromaRows)
        return img;
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            const quint8 lum = quint8(packed.at(y * w + x));
            const int uvIndex = pixels + (y / 2) * uvStride + (x & ~1);
            const quint8 u = quint8(packed.at(uvIndex));
            const quint8 v = quint8(packed.at(uvIndex + 1));
            int r, g, b;
            PixelFormatPacking::yuvToRgb(lum, u, v, &r, &g, &b);
            img.setPixel(x, y, qRgb(r, g, b));
        }
    }
    return img;
}

QImage previewFromYuvYv12(const QByteArray &packed, int w, int h)
{
    QImage img(w, h, QImage::Format_ARGB32);
    const int pixels = w * h;
    const int chromaWidth = (w + 1) / 2;
    const int chromaRows = (h + 1) / 2;
    const int chromaPixels = chromaWidth * chromaRows;
    if (packed.size() < pixels + chromaPixels * 2)
        return img;
    const int uOff = pixels;
    const int vOff = pixels + chromaPixels;
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            const quint8 lum = quint8(packed.at(y * w + x));
            const int ci = (y / 2) * chromaWidth + (x / 2);
            const quint8 u = quint8(packed.at(uOff + ci));
            const quint8 v = quint8(packed.at(vOff + ci));
            int r, g, b;
            PixelFormatPacking::yuvToRgb(lum, u, v, &r, &g, &b);
            img.setPixel(x, y, qRgb(r, g, b));
        }
    }
    return img;
}

float halfToFloat(quint16 value)
{
    const quint32 sign = quint32(value & 0x8000) << 16;
    quint32 exp = (value >> 10) & 0x1F;
    quint32 mant = value & 0x3FF;
    if (exp == 0) {
        if (mant == 0)
            return *reinterpret_cast<const float *>(&sign);
        exp = 1;
        while ((mant & 0x400) == 0) {
            mant <<= 1;
            --exp;
        }
        mant &= 0x3FF;
    } else if (exp == 31) {
        const quint32 bits = sign | 0x7F800000 | (mant << 13);
        return *reinterpret_cast<const float *>(&bits);
    }
    exp = exp + 127 - 15;
    const quint32 bits = sign | (exp << 23) | (mant << 13);
    return *reinterpret_cast<const float *>(&bits);
}

quint16 floatToHalf(float value)
{
    const quint32 bits = *reinterpret_cast<const quint32 *>(&value);
    const quint32 sign = (bits >> 16) & 0x8000;
    qint32 exp = qint32((bits >> 23) & 0xFF) - 127 + 15;
    quint32 mant = (bits >> 13) & 0x3FF;
    if (exp <= 0) {
        if (exp < -10)
            return quint16(sign);
        mant |= 0x400;
        mant >>= quint32(1 - exp);
        return quint16(sign | mant);
    }
    if (exp >= 31)
        return quint16(sign | 0x7C00);
    return quint16(sign | quint32(exp << 10) | mant);
}

QImage previewFromR16f(const QByteArray &grayscale8, int w, int h)
{
    QImage img(w, h, QImage::Format_ARGB32);
    if (grayscale8.size() < w * h)
        return img;
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            const float n = float(uchar(grayscale8.at(y * w + x))) / 255.0f;
            const quint16 half = floatToHalf(n);
            const float decoded = halfToFloat(half);
            const int g = qBound(0, int(decoded * 255.0f + 0.5f), 255);
            img.setPixel(x, y, qRgb(g, g, g));
        }
    }
    return img;
}

QImage previewFromRgba32f(const QByteArray &rgb888, int w, int h)
{
    QImage img(w, h, QImage::Format_ARGB32);
    const int pixels = w * h;
    if (rgb888.size() < pixels * 3)
        return img;
    for (int i = 0; i < pixels; ++i) {
        const int r = qBound(0, int(float(uchar(rgb888.at(i * 3))) / 255.0f * 255.0f + 0.5f), 255);
        const int g = qBound(0, int(float(uchar(rgb888.at(i * 3 + 1))) / 255.0f * 255.0f + 0.5f), 255);
        const int b = qBound(0, int(float(uchar(rgb888.at(i * 3 + 2))) / 255.0f * 255.0f + 0.5f), 255);
        img.setPixel(i % w, i / w, qRgb(r, g, b));
    }
    return img;
}

void buildIndexed8(DisplayRasterizer::Result &r, int width, int height)
{
    PixelFormatPacking::buildIndexedMedianCut(r.rgb888, width * height, r.rgb233, r.indexedPalette);
}

QImage previewFromRgb888(const QByteArray &packed, int w, int h)
{
    QImage img(w, h, QImage::Format_ARGB32);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            const int i = (y * w + x) * 3;
            img.setPixel(x, y, qRgb(uchar(packed.at(i)),
                                    uchar(packed.at(i + 1)),
                                    uchar(packed.at(i + 2))));
        }
    }
    return img;
}

QImage previewFromRgb24(const QVector<quint32> &pixels, int w, int h)
{
    QImage img(w, h, QImage::Format_ARGB32);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            const quint32 v = pixels.at(y * w + x);
            img.setPixel(x, y, qRgb((v >> 16) & 0xFF, (v >> 8) & 0xFF, v & 0xFF));
        }
    }
    return img;
}

ImageFiltersPipeline::OutputMode outputModeForEncoding(DisplayCodeGenerator::EncodingMode encoding)
{
    if (PixelFormatCatalog::isMono(encoding))
        return ImageFiltersPipeline::OutputMode::Mono1Bit;
    if (PixelFormatCatalog::isGrayscale(encoding))
        return ImageFiltersPipeline::OutputMode::Grayscale8;
    return ImageFiltersPipeline::OutputMode::Color;
}

bool rasterUsesMonoPath(DisplayCodeGenerator::EncodingMode encoding)
{
    return outputModeForEncoding(encoding) == ImageFiltersPipeline::OutputMode::Mono1Bit;
}

} // namespace

void DisplayRasterizer::buildIndexedPalette(Result &result)
{
    buildIndexed8(result, result.width, result.height);
}

void DisplayRasterizer::invertMonoResult(Result &result)
{
    if (result.monoBits.isEmpty() || result.width < 1 || result.height < 1)
        return;
    for (int i = 0; i < result.monoBits.size(); ++i)
        result.monoBits[i] = !result.monoBits[i];
    result.monoBuffer = packSsd1306(result.monoBits, result.width, result.height);
    result.preview = previewFromMono(result.monoBits, result.width, result.height);
}

DisplayRasterizer::Result DisplayRasterizer::convert(const QImage &source,
                                                     int width,
                                                     int height,
                                                     DisplayProfile::ColorMode colorMode,
                                                     DisplayProfile::ScaleMode scaleMode,
                                                     const ImageFiltersPipeline::Params &filters,
                                                     DisplayCodeGenerator::EncodingMode encodingMode,
                                                     int monoThreshold,
                                                     bool invertMono,
                                                     bool linearColorSpace)
{
    Result r;
    r.width = width;
    r.height = height;

    const QImage scaled = scaleToTarget(source, width, height, scaleMode);
    if (scaled.isNull())
        return r;

    ImageFiltersPipeline::Params pipeline = filters;
    pipeline.threshold = monoThreshold;
    pipeline.outputMode = outputModeForEncoding(encodingMode);
    if (pipeline.outputMode == ImageFiltersPipeline::OutputMode::Mono1Bit) {
        pipeline.thresholdEnabled = (pipeline.ditherMode == ImageFiltersPipeline::DitherMode::None);
    } else {
        pipeline.thresholdEnabled = false;
    }

    const QImage filtered = ImageFiltersPipeline::apply(scaled, pipeline);
    if (filtered.isNull())
        return r;
    r.processPreview = filtered;

    if (!rasterUsesMonoPath(encodingMode)) {
        r.rgb565.resize(width * height);
        r.rgb24.resize(width * height);
        r.rgb888.resize(width * height * 3);
        r.rgb233.resize(width * height);
        r.grayscale8.resize(width * height);
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                const QRgb pixel = filtered.pixel(x, y);
                const int i = y * width + x;
                const quint8 r8 = quint8(qRed(pixel));
                const quint8 g8 = quint8(qGreen(pixel));
                const quint8 b8 = quint8(qBlue(pixel));
                const float rLin = linearColorSpace
                    ? PixelFormatPacking::srgbChannelToLinear(float(r8) / 255.0f)
                    : float(r8) / 255.0f;
                const float gLin = linearColorSpace
                    ? PixelFormatPacking::srgbChannelToLinear(float(g8) / 255.0f)
                    : float(g8) / 255.0f;
                const float bLin = linearColorSpace
                    ? PixelFormatPacking::srgbChannelToLinear(float(b8) / 255.0f)
                    : float(b8) / 255.0f;
                r.rgb565[i] = PixelFormatPacking::quantizeLinearToRgb565(rLin, gLin, bLin);
                r.rgb24[i] = quint32(0xFF000000 | (quint32(r8) << 16) | (quint32(g8) << 8) | quint32(b8));
                r.rgb888[i * 3] = char(r8);
                r.rgb888[i * 3 + 1] = char(g8);
                r.rgb888[i * 3 + 2] = char(b8);
                r.rgb233[i] = char(toRgb233(pixel));
                const float yLin = 0.299f * rLin + 0.587f * gLin + 0.114f * bLin;
                r.grayscale8[i] = char(PixelFormatPacking::quantizeLinearToGray8(yLin));
            }
        }
        if (encodingMode == DisplayCodeGenerator::EncodingMode::Indexed8)
            buildIndexed8(r, width, height);
        r.preview = previewFromRgb565(r.rgb565, width, height);
        applyPreviewEncoding(r, encodingMode);
        return r;
    }

    const int thr = qBound(0, monoThreshold, 255);
    QVector<bool> bits(width * height);
    if (filters.ditherMode == ImageFiltersPipeline::DitherMode::FloydSteinberg) {
        bits = monoBitsFloydSteinberg(filtered, thr);
    } else {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                bits[y * width + x] = grayValue(filtered.pixel(x, y)) >= thr;
            }
        }
    }
    r.monoBits = bits;
    r.monoBuffer = packSsd1306(bits, width, height);
    r.preview = previewFromMono(bits, width, height);
    if (invertMono)
        invertMonoResult(r);
    return r;
}

void DisplayRasterizer::applyPreviewEncoding(Result &result, DisplayCodeGenerator::EncodingMode encodingMode)
{
    if (result.width < 1 || result.height < 1)
        return;

    const int w = result.width;
    const int h = result.height;

    using EM = DisplayCodeGenerator::EncodingMode;
    switch (encodingMode) {
    case EM::Grayscale4:
        if (result.grayscale8.size() >= w * h)
            result.preview = previewFromGrayscale4(result.grayscale8, w, h);
        break;
    case EM::Grayscale8:
        if (result.grayscale8.size() >= w * h)
            result.preview = previewFromGrayscale8(result.grayscale8, w, h);
        break;
    case EM::Indexed8:
        if (result.indexedPalette.isEmpty() && result.rgb888.size() >= w * h * 3)
            buildIndexed8(result, w, h);
        if (result.rgb233.size() >= w * h && !result.indexedPalette.isEmpty())
            result.preview = previewFromIndexed8(result.rgb233, result.indexedPalette, w, h);
        break;
    case EM::Rgb888:
        if (result.rgb888.size() >= w * h * 3)
            result.preview = previewFromRgb888(result.rgb888, w, h);
        break;
    case EM::Bgr888:
        if (result.rgb888.size() >= w * h * 3)
            result.preview = previewFromBgr888(result.rgb888, w, h);
        break;
    case EM::Rgb666: {
        if (result.rgb888.size() < w * h * 3)
            break;
        const QByteArray packed = PixelFormatPacking::packRgb666Compact(result.rgb888, w * h);
        QByteArray decoded;
        PixelFormatPacking::decodeRgb666Compact(packed, w * h, decoded);
        if (decoded.size() >= w * h * 3)
            result.preview = previewFromRgb888(decoded, w, h);
        break;
    }
    case EM::Argb8888:
    case EM::Abgr8888:
        if (result.rgb24.size() >= w * h)
            result.preview = previewFromRgb24(result.rgb24, w, h);
        break;
    case EM::Rgb565:
        if (!result.rgb565.isEmpty())
            result.preview = previewFromRgb565(result.rgb565, w, h);
        break;
    case EM::Bgr565:
        if (!result.rgb565.isEmpty())
            result.preview = previewFromBgr565(result.rgb565, w, h);
        break;
    case EM::YuvYuyv:
    case EM::YuvNv12:
    case EM::YuvYv12: {
        if (result.rgb888.size() < w * h * 3)
            break;
        const QByteArray packed = encodingMode == EM::YuvYuyv
            ? PixelFormatPacking::packYuvYuyv(result.rgb888, w, h)
            : (encodingMode == EM::YuvNv12
                   ? PixelFormatPacking::packYuvNv12(result.rgb888, w, h)
                   : PixelFormatPacking::packYuvYv12(result.rgb888, w, h));
        if (encodingMode == EM::YuvYuyv)
            result.preview = previewFromYuvYuyv(packed, w, h);
        else if (encodingMode == EM::YuvNv12)
            result.preview = previewFromYuvNv12(packed, w, h);
        else
            result.preview = previewFromYuvYv12(packed, w, h);
        break;
    }
    case EM::R16f:
        if (result.grayscale8.size() >= w * h)
            result.preview = previewFromR16f(result.grayscale8, w, h);
        break;
    case EM::Rgba32f:
        if (result.rgb888.size() >= w * h * 3)
            result.preview = previewFromRgba32f(result.rgb888, w, h);
        break;
    default:
        break;
    }
}
