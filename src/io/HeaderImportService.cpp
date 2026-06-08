#include "io/HeaderImportService.h"

#include "translation/AppLocale.h"

#include <QFile>
#include <QRegularExpression>
#include <QVector>

namespace {

QVector<int> parseNumbers(const QString &text)
{
    QVector<int> values;
    QRegularExpression re(QStringLiteral("0x([0-9a-fA-F]+)|\\b(\\d+)\\b"));
    QRegularExpressionMatchIterator it = re.globalMatch(text);
    while (it.hasNext()) {
        const QRegularExpressionMatch m = it.next();
        bool ok = false;
        const int value = !m.captured(1).isEmpty()
            ? m.captured(1).toInt(&ok, 16)
            : m.captured(2).toInt(&ok, 10);
        if (ok)
            values.append(value);
    }
    return values;
}

int macroValue(const QString &text, const QString &suffix)
{
    const QRegularExpression re(QStringLiteral("#define\\s+\\w+_%1\\s+(\\d+)").arg(suffix));
    const QRegularExpressionMatch m = re.match(text);
    return m.hasMatch() ? m.captured(1).toInt() : 0;
}

QString arrayName(const QString &text)
{
    const QRegularExpression re(QStringLiteral("(?:const|static).*?\\s+(\\w+)\\s*\\[\\s*\\]"));
    const QRegularExpressionMatch m = re.match(text);
    return m.hasMatch() ? m.captured(1) : QStringLiteral("imported_image");
}

QImage monoPreview(const QVector<int> &bytes, int width, int height)
{
    QImage img(width, height, QImage::Format_ARGB32);
    img.fill(Qt::black);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const int byteIndex = (y * width + x) / 8;
            const int bit = 7 - ((y * width + x) % 8);
            if (byteIndex < bytes.size() && (bytes[byteIndex] & (1 << bit)))
                img.setPixel(x, y, qRgb(255, 255, 255));
        }
    }
    return img;
}

QImage rgb565Preview(const QVector<int> &values, int width, int height)
{
    QImage img(width, height, QImage::Format_ARGB32);
    for (int i = 0; i < width * height; ++i) {
        const quint16 v = i < values.size() ? quint16(values[i]) : 0;
        const int r = ((v >> 11) & 0x1F) * 255 / 31;
        const int g = ((v >> 5) & 0x3F) * 255 / 63;
        const int b = (v & 0x1F) * 255 / 31;
        img.setPixel(i % width, i / width, qRgb(r, g, b));
    }
    return img;
}

} // namespace

HeaderImportResult HeaderImportService::importHeader(const QUrl &url)
{
    HeaderImportResult out;
    QString path = url.toLocalFile();
    if (path.isEmpty())
        path = url.path();
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        out.errorMessage = AppLocale::tr("Failed to open header: %1").arg(path);
        return out;
    }
    const QString text = QString::fromUtf8(file.readAll());
    out.arrayName = arrayName(text);
    out.width = macroValue(text, QStringLiteral("WIDTH"));
    out.height = macroValue(text, QStringLiteral("HEIGHT"));
    if (out.width <= 0 || out.height <= 0) {
        out.errorMessage = AppLocale::tr("Could not determine WIDTH/HEIGHT");
        return out;
    }
    const QVector<int> values = parseNumbers(text.mid(text.indexOf(QLatin1Char('{'))));
    const bool rgb = text.contains(QStringLiteral("uint16_t")) || values.size() >= out.width * out.height;
    out.colorMode = rgb ? DisplayProfile::Rgb565 : DisplayProfile::Mono1Bit;
    out.encodingMode = rgb ? DisplayCodeGenerator::EncodingMode::Rgb565 : DisplayCodeGenerator::EncodingMode::Mono1Bit;
    out.preview = rgb ? rgb565Preview(values, out.width, out.height) : monoPreview(values, out.width, out.height);
    out.ok = !out.preview.isNull();
    return out;
}
