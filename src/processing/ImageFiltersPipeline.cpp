#include "processing/ImageFiltersPipeline.h"

#include <cmath>

#include <QtMath>

namespace {

int clamp255(int value)
{
    return qBound(0, value, 255);
}

QImage ensureArgb(const QImage &source)
{
    if (source.format() == QImage::Format_ARGB32)
        return source;
    return source.convertToFormat(QImage::Format_ARGB32);
}

void applyBlackBackground(QImage &img)
{
    for (int y = 0; y < img.height(); ++y) {
        QRgb *row = reinterpret_cast<QRgb *>(img.scanLine(y));
        for (int x = 0; x < img.width(); ++x) {
            const int a = qAlpha(row[x]);
            if (a >= 255)
                continue;
            const int r = (qRed(row[x]) * a) / 255;
            const int g = (qGreen(row[x]) * a) / 255;
            const int b = (qBlue(row[x]) * a) / 255;
            row[x] = qRgba(r, g, b, 255);
        }
    }
}

void applyExposureGamma(QImage &img, int exposure, int gamma)
{
    if (exposure == 100 && gamma == 100)
        return;
    const qreal expMul = qreal(exposure) / 100.0;
    const qreal gammaExp = 100.0 / qMax(25, gamma);

    for (int y = 0; y < img.height(); ++y) {
        QRgb *row = reinterpret_cast<QRgb *>(img.scanLine(y));
        for (int x = 0; x < img.width(); ++x) {
            qreal r = qRed(row[x]) / 255.0;
            qreal g = qGreen(row[x]) / 255.0;
            qreal b = qBlue(row[x]) / 255.0;
            r = 255.0 * std::pow(qBound(0.0, r, 1.0), gammaExp) * expMul;
            g = 255.0 * std::pow(qBound(0.0, g, 1.0), gammaExp) * expMul;
            b = 255.0 * std::pow(qBound(0.0, b, 1.0), gammaExp) * expMul;
            row[x] = qRgba(clamp255(qRound(r)), clamp255(qRound(g)), clamp255(qRound(b)), 255);
        }
    }
}

void applyColorAdjustments(QImage &img, int brightness, int contrast, int saturation)
{
    const qreal bright = qreal(brightness - 100) * 2.55;
    const qreal contrastFactor = qBound(0.0, qreal(contrast) / 100.0, 3.0);
    const qreal satFactor = qBound(0.0, qreal(saturation) / 100.0, 3.0);

    for (int y = 0; y < img.height(); ++y) {
        QRgb *row = reinterpret_cast<QRgb *>(img.scanLine(y));
        for (int x = 0; x < img.width(); ++x) {
            qreal r = qRed(row[x]);
            qreal g = qGreen(row[x]);
            qreal b = qBlue(row[x]);

            r = ((r - 127.5) * contrastFactor) + 127.5 + bright;
            g = ((g - 127.5) * contrastFactor) + 127.5 + bright;
            b = ((b - 127.5) * contrastFactor) + 127.5 + bright;

            const qreal gray = 0.299 * r + 0.587 * g + 0.114 * b;
            r = gray + (r - gray) * satFactor;
            g = gray + (g - gray) * satFactor;
            b = gray + (b - gray) * satFactor;

            row[x] = qRgba(clamp255(qRound(r)), clamp255(qRound(g)), clamp255(qRound(b)), 255);
        }
    }
}

QImage boxBlur(const QImage &img, int radius)
{
    if (radius < 1)
        return img;
    QImage out(img.size(), QImage::Format_ARGB32);
    for (int y = 0; y < img.height(); ++y) {
        for (int x = 0; x < img.width(); ++x) {
            int sr = 0;
            int sg = 0;
            int sb = 0;
            int count = 0;
            for (int ky = -radius; ky <= radius; ++ky) {
                const int py = qBound(0, y + ky, img.height() - 1);
                for (int kx = -radius; kx <= radius; ++kx) {
                    const int px = qBound(0, x + kx, img.width() - 1);
                    const QRgb c = img.pixel(px, py);
                    sr += qRed(c);
                    sg += qGreen(c);
                    sb += qBlue(c);
                    ++count;
                }
            }
            out.setPixel(x, y, qRgba(sr / count, sg / count, sb / count, 255));
        }
    }
    return out;
}

void posterizeRgb(QImage &img, int value)
{
    if (value < 1)
        return;
    const int levels = qBound(2, value + 2, 32);
    const int step = qMax(1, 256 / levels);
    for (int y = 0; y < img.height(); ++y) {
        QRgb *row = reinterpret_cast<QRgb *>(img.scanLine(y));
        for (int x = 0; x < img.width(); ++x) {
            const int r = (qRed(row[x]) / step) * step;
            const int g = (qGreen(row[x]) / step) * step;
            const int b = (qBlue(row[x]) / step) * step;
            row[x] = qRgba(clamp255(r), clamp255(g), clamp255(b), 255);
        }
    }
}

QImage colorMask(const QImage &img, const QColor &target, int tolerance, int amplify)
{
    QImage out(img.size(), QImage::Format_ARGB32);
    const int tol = qMax(0, tolerance);
    const int amp = qMax(1, amplify);
    for (int y = 0; y < img.height(); ++y) {
        for (int x = 0; x < img.width(); ++x) {
            const QRgb c = img.pixel(x, y);
            const int dr = qRed(c) - target.red();
            const int dg = qGreen(c) - target.green();
            const int db = qBlue(c) - target.blue();
            const qreal dist = qSqrt(qreal(dr * dr + dg * dg + db * db));
            qreal mask = 255.0 - dist * amp;
            mask += tol;
            const int g = clamp255(qRound(mask));
            out.setPixel(x, y, qRgba(g, g, g, 255));
        }
    }
    return out;
}

QImage toGray(const QImage &img)
{
    return img.convertToFormat(QImage::Format_Grayscale8).convertToFormat(QImage::Format_ARGB32);
}

QImage convolve3(const QImage &img, const int k[9], int div, int bias)
{
    QImage out(img.size(), QImage::Format_ARGB32);
    for (int y = 0; y < img.height(); ++y) {
        for (int x = 0; x < img.width(); ++x) {
            int sum = 0;
            int idx = 0;
            for (int ky = -1; ky <= 1; ++ky) {
                const int py = qBound(0, y + ky, img.height() - 1);
                for (int kx = -1; kx <= 1; ++kx) {
                    const int px = qBound(0, x + kx, img.width() - 1);
                    sum += qGray(img.pixel(px, py)) * k[idx++];
                }
            }
            const int v = clamp255((sum / qMax(1, div)) + bias);
            out.setPixel(x, y, qRgba(v, v, v, 255));
        }
    }
    return out;
}

QImage sobelEdges(const QImage &img, int power)
{
    if (power <= 0)
        return img;
    QImage out(img.size(), QImage::Format_ARGB32);
    for (int y = 0; y < img.height(); ++y) {
        for (int x = 0; x < img.width(); ++x) {
            const int x0 = qBound(0, x - 1, img.width() - 1);
            const int x1 = qBound(0, x + 1, img.width() - 1);
            const int y0 = qBound(0, y - 1, img.height() - 1);
            const int y1 = qBound(0, y + 1, img.height() - 1);

            const int gx =
                -qGray(img.pixel(x0, y0)) + qGray(img.pixel(x1, y0)) +
                -2 * qGray(img.pixel(x0, y)) + 2 * qGray(img.pixel(x1, y)) +
                -qGray(img.pixel(x0, y1)) + qGray(img.pixel(x1, y1));
            const int gy =
                qGray(img.pixel(x0, y0)) + 2 * qGray(img.pixel(x, y0)) + qGray(img.pixel(x1, y0)) -
                qGray(img.pixel(x0, y1)) - 2 * qGray(img.pixel(x, y1)) - qGray(img.pixel(x1, y1));
            const int mag = clamp255(qRound(qSqrt(qreal(gx * gx + gy * gy)) * (qreal(power) / 100.0)));
            out.setPixel(x, y, qRgba(mag, mag, mag, 255));
        }
    }
    return out;
}

void posterizeGray(QImage &img, int value)
{
    if (value < 1)
        return;
    const int levels = qBound(2, value + 2, 32);
    const int step = qMax(1, 256 / levels);
    for (int y = 0; y < img.height(); ++y) {
        QRgb *row = reinterpret_cast<QRgb *>(img.scanLine(y));
        for (int x = 0; x < img.width(); ++x) {
            const int g = (qGray(row[x]) / step) * step;
            row[x] = qRgba(g, g, g, 255);
        }
    }
}

QImage bayerDither(const QImage &img)
{
    static const int matrix[4][4] = {
        {0, 8, 2, 10},
        {12, 4, 14, 6},
        {3, 11, 1, 9},
        {15, 7, 13, 5}
    };
    QImage out(img.size(), QImage::Format_ARGB32);
    for (int y = 0; y < img.height(); ++y) {
        for (int x = 0; x < img.width(); ++x) {
            const int g = qGray(img.pixel(x, y));
            const int t = matrix[y % 4][x % 4] * 16;
            const int v = g > t ? 255 : 0;
            out.setPixel(x, y, qRgba(v, v, v, 255));
        }
    }
    return out;
}

QImage floydDither(const QImage &img)
{
    const int w = img.width();
    const int h = img.height();
    QVector<qreal> values(w * h);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x)
            values[y * w + x] = qGray(img.pixel(x, y));
    }
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            const int idx = y * w + x;
            const qreal old = values[idx];
            const qreal next = old > 127 ? 255 : 0;
            const qreal err = old - next;
            values[idx] = next;
            if (x + 1 < w)
                values[idx + 1] += err * 7.0 / 16.0;
            if (y + 1 < h) {
                if (x > 0)
                    values[idx + w - 1] += err * 3.0 / 16.0;
                values[idx + w] += err * 5.0 / 16.0;
                if (x + 1 < w)
                    values[idx + w + 1] += err * 1.0 / 16.0;
            }
        }
    }
    QImage out(w, h, QImage::Format_ARGB32);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            const int v = values[y * w + x] > 127 ? 255 : 0;
            out.setPixel(x, y, qRgba(v, v, v, 255));
        }
    }
    return out;
}

QImage jjnDither(const QImage &img)
{
    const int w = img.width();
    const int h = img.height();
    QVector<qreal> values(w * h);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x)
            values[y * w + x] = qGray(img.pixel(x, y));
    }
    auto addErr = [&](int x, int y, qreal err, qreal weight) {
        if (x < 0 || y < 0 || x >= w || y >= h)
            return;
        values[y * w + x] += err * weight / 48.0;
    };
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            const int idx = y * w + x;
            const qreal old = values[idx];
            const qreal next = old > 127 ? 255 : 0;
            const qreal err = old - next;
            values[idx] = next;
            addErr(x + 1, y, err, 7);
            addErr(x + 2, y, err, 5);
            addErr(x - 2, y + 1, err, 3);
            addErr(x - 1, y + 1, err, 5);
            addErr(x, y + 1, err, 7);
            addErr(x + 1, y + 1, err, 5);
            addErr(x + 2, y + 1, err, 3);
            addErr(x - 2, y + 2, err, 1);
            addErr(x - 1, y + 2, err, 3);
            addErr(x, y + 2, err, 5);
            addErr(x + 1, y + 2, err, 3);
            addErr(x + 2, y + 2, err, 1);
        }
    }
    QImage out(w, h, QImage::Format_ARGB32);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            const int v = values[y * w + x] > 127 ? 255 : 0;
            out.setPixel(x, y, qRgba(v, v, v, 255));
        }
    }
    return out;
}

void thresholdMono(QImage &img, int threshold)
{
    const int thr = qBound(0, threshold, 255);
    for (int y = 0; y < img.height(); ++y) {
        QRgb *row = reinterpret_cast<QRgb *>(img.scanLine(y));
        for (int x = 0; x < img.width(); ++x) {
            const int v = qGray(row[x]) >= thr ? 255 : 0;
            row[x] = qRgba(v, v, v, 255);
        }
    }
}

QImage contourize(const QImage &img, ImageFiltersPipeline::ContourMode mode)
{
    if (mode == ImageFiltersPipeline::ContourMode::None)
        return img;
    QImage out(img.size(), QImage::Format_ARGB32);
    const bool checkDiag = mode == ImageFiltersPipeline::ContourMode::EightDir;
    for (int y = 0; y < img.height(); ++y) {
        for (int x = 0; x < img.width(); ++x) {
            const int base = qGray(img.pixel(x, y));
            bool edge = false;
            auto diff = [&](int px, int py) {
                if (px < 0 || py < 0 || px >= img.width() || py >= img.height())
                    return;
                if (qAbs(base - qGray(img.pixel(px, py))) > 16)
                    edge = true;
            };
            diff(x - 1, y);
            diff(x + 1, y);
            diff(x, y - 1);
            diff(x, y + 1);
            if (checkDiag) {
                diff(x - 1, y - 1);
                diff(x + 1, y - 1);
                diff(x - 1, y + 1);
                diff(x + 1, y + 1);
            }
            const int v = edge ? 255 : 0;
            out.setPixel(x, y, qRgba(v, v, v, 255));
        }
    }
    return out;
}

void invert(QImage &img)
{
    for (int y = 0; y < img.height(); ++y) {
        QRgb *row = reinterpret_cast<QRgb *>(img.scanLine(y));
        for (int x = 0; x < img.width(); ++x) {
            row[x] = qRgba(255 - qRed(row[x]), 255 - qGreen(row[x]), 255 - qBlue(row[x]), 255);
        }
    }
}

} // namespace

ImageFiltersPipeline::Params ImageFiltersPipeline::paramsForTonePreset(TonePreset preset)
{
    Params p;
    p.tonePreset = preset;
    if (preset == TonePreset::Icon) {
        p.brightness = 108;
        p.contrast = 135;
        p.saturation = 85;
        p.exposure = 105;
        p.gamma = 115;
        p.posterizeGray = 4;
    } else if (preset == TonePreset::Photo) {
        p.brightness = 102;
        p.contrast = 110;
        p.saturation = 115;
        p.exposure = 108;
        p.gamma = 95;
    }
    return p;
}

bool ImageFiltersPipeline::Params::usesGrayPipeline() const
{
    if (outputMode == OutputMode::Grayscale8) {
        return sharpen
            || sobelEdges > 0
            || posterizeGray > 0
            || contourMode != ContourMode::None;
    }
    if (outputMode == OutputMode::Color) {
        return colorMaskEnabled
            || sharpen
            || sobelEdges > 0
            || posterizeGray > 0;
    }
    return colorMaskEnabled
        || sharpen
        || sobelEdges > 0
        || posterizeGray > 0
        || ditherMode != DitherMode::None
        || thresholdEnabled
        || contourMode != ContourMode::None
        || invert;
}

QImage ImageFiltersPipeline::apply(const QImage &source, const Params &params)
{
    QImage img = ensureArgb(source);
    if (params.blackBackground)
        applyBlackBackground(img);

    if (params.brightness != 100 || params.contrast != 100 || params.saturation != 100)
        applyColorAdjustments(img, params.brightness, params.contrast, params.saturation);

    if (params.exposure != 100 || params.gamma != 100)
        applyExposureGamma(img, params.exposure, params.gamma);

    if (params.blur > 0)
        img = boxBlur(img, qBound(0, params.blur, 6));

    if (params.posterizeRgb > 0 && params.outputMode == OutputMode::Color)
        posterizeRgb(img, params.posterizeRgb);

    if (params.colorMaskEnabled) {
        img = colorMask(img, params.maskColor, params.maskTolerance, params.maskAmplify);
    } else if (params.usesGrayPipeline()) {
        img = toGray(img);
    }

    if (params.sharpen) {
        const int kernel[9] = { 0, -1, 0, -1, 5, -1, 0, -1, 0 };
        img = convolve3(img, kernel, 1, 0);
    }

    if (params.sobelEdges > 0)
        img = sobelEdges(img, qBound(0, params.sobelEdges, 100));

    if (params.posterizeGray > 0)
        posterizeGray(img, params.posterizeGray);

    const bool monoBinary = params.outputMode == OutputMode::Mono1Bit;
    if (monoBinary) {
        if (params.ditherMode == DitherMode::FloydSteinberg)
            img = floydDither(img);
        else if (params.ditherMode == DitherMode::Jjn)
            img = jjnDither(img);
        else if (params.ditherMode == DitherMode::Bayer)
            img = bayerDither(img);
    }

    if (monoBinary
        && (params.thresholdEnabled || params.ditherMode != DitherMode::None
            || params.contourMode != ContourMode::None)) {
        thresholdMono(img, params.threshold);
        img = contourize(img, params.contourMode);
    }

    if (params.invert)
        invert(img);

    return img;
}
