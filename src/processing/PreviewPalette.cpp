#include "processing/PreviewPalette.h"

#include <QSet>

namespace PreviewPalette {

namespace {

QString rgbToHex(int r, int g, int b)
{
    return QStringLiteral("#%1%2%3")
        .arg(r, 2, 16, QChar('0'))
        .arg(g, 2, 16, QChar('0'))
        .arg(b, 2, 16, QChar('0'))
        .toUpper();
}

} // namespace

QVariantList fromRgbList(const QVector<quint32> &colors)
{
    QVariantList out;
    out.reserve(colors.size());
    for (quint32 c : colors) {
        out.append(rgbToHex(int((c >> 16) & 0xFF), int((c >> 8) & 0xFF), int(c & 0xFF)));
    }
    return out;
}

QVariantList fromPreviewImage(const QImage &img, int maxColors)
{
    QVariantList out;
    if (img.isNull() || maxColors < 1)
        return out;

    const QImage sample = (img.width() > 160 || img.height() > 160)
        ? img.scaled(160, 160, Qt::KeepAspectRatio, Qt::FastTransformation)
        : img;

    QSet<quint32> seen;
    QVector<quint32> ordered;
    ordered.reserve(qMin(maxColors, sample.width() * sample.height()));

    for (int y = 0; y < sample.height(); ++y) {
        for (int x = 0; x < sample.width(); ++x) {
            const QRgb px = sample.pixel(x, y);
            const int a = qAlpha(px);
            if (a < 8)
                continue;
            const quint32 key = quint32(px) | 0xFF000000U;
            if (seen.contains(key))
                continue;
            seen.insert(key);
            ordered.append(key);
            if (ordered.size() >= maxColors)
                return fromRgbList(ordered);
        }
    }
    return fromRgbList(ordered);
}

} // namespace PreviewPalette
