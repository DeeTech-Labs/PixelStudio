#include "processing/DisplayRasterizer.h"
#include <QPainter>
#include <QtMath>

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

QRgb fromRgb233(quint8 v)
{
    const int r = ((v >> 5) & 0x07) * 255 / 7;
    const int g = ((v >> 2) & 0x07) * 255 / 7;
    const int b = (v & 0x03) * 255 / 3;
    return qRgb(r, g, b);
}

QImage previewFromRgb233(const QByteArray &packed, int w, int h)
{
    QImage img(w, h, QImage::Format_ARGB32);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            img.setPixel(x, y, fromRgb233(quint8(packed.at(y * w + x))));
        }
    }
    return img;
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
    using EM = DisplayCodeGenerator::EncodingMode;
    const auto mode = static_cast<EM>(encoding);
    if (mode <= EM::PackedImageRle || mode == EM::Ascii || mode == EM::Bricks)
        return ImageFiltersPipeline::OutputMode::Mono1Bit;
    if (mode == EM::Grayscale8)
        return ImageFiltersPipeline::OutputMode::Grayscale8;
    return ImageFiltersPipeline::OutputMode::Color;
}

bool rasterUsesMonoPath(DisplayCodeGenerator::EncodingMode encoding)
{
    return outputModeForEncoding(encoding) == ImageFiltersPipeline::OutputMode::Mono1Bit;
}

} // namespace

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
                                                     bool invertMono)
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
                r.rgb565[i] = toRgb565(pixel);
                r.rgb24[i] = quint32((quint32(qRed(pixel)) << 16)
                                   | (quint32(qGreen(pixel)) << 8)
                                   | quint32(qBlue(pixel)));
                r.rgb888[i * 3] = char(qRed(pixel));
                r.rgb888[i * 3 + 1] = char(qGreen(pixel));
                r.rgb888[i * 3 + 2] = char(qBlue(pixel));
                r.rgb233[i] = char(toRgb233(pixel));
                r.grayscale8[i] = char(qGray(pixel));
            }
        }
        r.preview = previewFromRgb565(r.rgb565, width, height);
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

    switch (encodingMode) {
    case DisplayCodeGenerator::EncodingMode::Grayscale8:
        if (result.grayscale8.size() >= w * h)
            result.preview = previewFromGrayscale8(result.grayscale8, w, h);
        break;
    case DisplayCodeGenerator::EncodingMode::Rgb233:
        if (result.rgb233.size() >= w * h)
            result.preview = previewFromRgb233(result.rgb233, w, h);
        break;
    case DisplayCodeGenerator::EncodingMode::Rgb888:
        if (result.rgb888.size() >= w * h * 3)
            result.preview = previewFromRgb888(result.rgb888, w, h);
        break;
    case DisplayCodeGenerator::EncodingMode::Rgb24:
        if (result.rgb24.size() >= w * h)
            result.preview = previewFromRgb24(result.rgb24, w, h);
        break;
    case DisplayCodeGenerator::EncodingMode::Rgb565:
        if (!result.rgb565.isEmpty())
            result.preview = previewFromRgb565(result.rgb565, w, h);
        break;
    default:
        break;
    }
}
