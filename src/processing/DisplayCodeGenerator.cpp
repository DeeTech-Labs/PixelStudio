#include "processing/DisplayCodeGenerator.h"
#include <QStringList>
#include <QVariantMap>

namespace {

QByteArray packMono1PixPerByte(const QVector<bool> &bits)
{
    QByteArray out(bits.size(), 0);
    for (int i = 0; i < bits.size(); ++i)
        out[i] = bits[i] ? char(0x01) : char(0x00);
    return out;
}

QByteArray packMonoHorizontal(const QVector<bool> &bits, int width, int height, bool msbLeft)
{
    const int bytesPerRow = (width + 7) / 8;
    QByteArray out(bytesPerRow * height, 0);
    for (int y = 0; y < height; ++y) {
        for (int bx = 0; bx < bytesPerRow; ++bx) {
            quint8 byte = 0;
            for (int bit = 0; bit < 8; ++bit) {
                const int x = bx * 8 + bit;
                if (x >= width)
                    break;
                if (bits[y * width + x]) {
                    const int pos = msbLeft ? (7 - bit) : bit;
                    byte |= quint8(1u << pos);
                }
            }
            out[y * bytesPerRow + bx] = char(byte);
        }
    }
    return out;
}

QByteArray packMonoVerticalCol(const QVector<bool> &bits, int width, int height)
{
    const int pages = (height + 7) / 8;
    QByteArray out(width * pages, 0);
    for (int x = 0; x < width; ++x) {
        for (int page = 0; page < pages; ++page) {
            quint8 byte = 0;
            for (int bit = 0; bit < 8; ++bit) {
                const int y = page * 8 + bit;
                if (y >= height)
                    break;
                if (bits[y * width + x])
                    byte |= quint8(1u << bit);
            }
            out[x * pages + page] = char(byte);
        }
    }
    return out;
}

QByteArray packMonoVerticalRow(const QVector<bool> &bits, int width, int height)
{
    const int pages = (height + 7) / 8;
    QByteArray out(width * pages, 0);
    for (int page = 0; page < pages; ++page) {
        for (int x = 0; x < width; ++x) {
            quint8 byte = 0;
            for (int bit = 0; bit < 8; ++bit) {
                const int y = page * 8 + bit;
                if (y >= height)
                    break;
                if (bits[y * width + x])
                    byte |= quint8(1u << bit);
            }
            out[page * width + x] = char(byte);
        }
    }
    return out;
}

QByteArray packHeaderBitmap(const QVector<bool> &bits, int width, int height)
{
    QByteArray out;
    out.reserve(4 + ((width * height + 7) / 8));
    out.append(char(width & 0xFF));
    out.append(char((width >> 8) & 0xFF));
    out.append(char(height & 0xFF));
    out.append(char((height >> 8) & 0xFF));
    out.append(packMonoVerticalCol(bits, width, height));
    return out;
}

QByteArray packHeaderRle(const QVector<bool> &bits, int width, int height)
{
    const QByteArray bitmap = packHeaderBitmap(bits, width, height);
    QByteArray out;
    out.reserve(bitmap.size());
    out.append(bitmap.left(4));
    int i = 4;
    while (i < bitmap.size()) {
        const char value = bitmap.at(i);
        int run = 1;
        while (i + run < bitmap.size() && bitmap.at(i + run) == value && run < 255)
            ++run;
        out.append(char(run));
        out.append(value);
        i += run;
    }
    return out;
}

QByteArray packAutoPackedImage(const QVector<bool> &bits, int width, int height)
{
    const QByteArray bitmap = packHeaderBitmap(bits, width, height);
    const QByteArray bitpack = packHeaderRle(bits, width, height);
    QByteArray out;
    if (bitpack.size() < bitmap.size()) {
        out.append(char(1));
        out.append(bitpack);
    } else {
        out.append(char(0));
        out.append(bitmap);
    }
    return out;
}

QString toHexByte(quint8 value)
{
    return QStringLiteral("0x%1").arg(value, 2, 16, QChar('0'));
}

bool monoBitsReady(const QVector<bool> &bits, int width, int height)
{
    return width > 0 && height > 0 && bits.size() == width * height;
}

int expectedBinarySize(DisplayCodeGenerator::EncodingMode mode,
                       int width,
                       int height,
                       DisplayCodeGenerator::MonoLayout monoLayout)
{
    const int pixels = width * height;
    const int pages = (height + 7) / 8;
    const int bytesPerRow = (width + 7) / 8;
    switch (mode) {
    case DisplayCodeGenerator::EncodingMode::Mono1PixPerByte:
        return pixels;
    case DisplayCodeGenerator::EncodingMode::Mono8HorizontalLsb:
    case DisplayCodeGenerator::EncodingMode::Mono8HorizontalMsb:
        if (monoLayout == DisplayCodeGenerator::MonoLayout::Ssd1306Page)
            return width * pages;
        return bytesPerRow * height;
    case DisplayCodeGenerator::EncodingMode::Mono8VerticalCol:
    case DisplayCodeGenerator::EncodingMode::Mono8VerticalRow:
        return width * pages;
    case DisplayCodeGenerator::EncodingMode::PackedImageHeader:
        return 4 + width * pages;
    case DisplayCodeGenerator::EncodingMode::PackedImageRle:
        return 4 + 2 * width * pages;
    case DisplayCodeGenerator::EncodingMode::PackedImageAuto:
        return 1 + 4 + width * pages;
    case DisplayCodeGenerator::EncodingMode::Grayscale8:
        return pixels;
    case DisplayCodeGenerator::EncodingMode::Rgb888:
        return pixels * 3;
    case DisplayCodeGenerator::EncodingMode::Rgb233:
        return pixels;
    case DisplayCodeGenerator::EncodingMode::Rgb565:
        return pixels * 2;
    case DisplayCodeGenerator::EncodingMode::Rgb24:
        return pixels * 4;
    case DisplayCodeGenerator::EncodingMode::Ascii:
    case DisplayCodeGenerator::EncodingMode::Bricks:
        return 0;
    }
    if (monoLayout == DisplayCodeGenerator::MonoLayout::Ssd1306Page)
        return width * pages;
    return bytesPerRow * height;
}

QByteArray placeholderBinary(DisplayCodeGenerator::EncodingMode mode,
                             int width,
                             int height,
                             DisplayCodeGenerator::MonoLayout monoLayout)
{
    return QByteArray(expectedBinarySize(mode, width, height, monoLayout), 0);
}

QString arrayDeclLine(const QString &type, const QString &name, const DisplayCodeGenerator::CodeGenOptions &opt)
{
    QString decl;
    if (opt.staticStorage)
        decl += QStringLiteral("static ");
    decl += QStringLiteral("const %1 %2[]").arg(type, name);
    if (opt.useProgmem)
        decl += QStringLiteral(" PROGMEM");
    return decl;
}

QString byteArrayToC(const QByteArray &data,
                     const QString &name,
                     const QString &type,
                     const DisplayCodeGenerator::CodeGenOptions &opt)
{
    QString out;
    out += arrayDeclLine(type, name, opt) + QStringLiteral(" = {\n");
    for (int i = 0; i < data.size(); ++i) {
        if (i % 16 == 0)
            out += QStringLiteral("    ");
        out += toHexByte(quint8(data.at(i)));
        if (i < data.size() - 1)
            out += QStringLiteral(", ");
        if (i % 16 == 15 || i == data.size() - 1)
            out += QLatin1Char('\n');
    }
    out += QStringLiteral("};\n");
    return out;
}

QString wordsToC(const QVector<quint16> &data,
                 const QString &name,
                 const DisplayCodeGenerator::CodeGenOptions &opt)
{
    QString out;
    out += arrayDeclLine(QStringLiteral("uint16_t"), name, opt) + QStringLiteral(" = {\n");
    for (int i = 0; i < data.size(); ++i) {
        if (i % 12 == 0)
            out += QStringLiteral("    ");
        out += QStringLiteral("0x%1").arg(data[i], 4, 16, QChar('0'));
        if (i < data.size() - 1)
            out += QStringLiteral(", ");
        if (i % 12 == 11 || i == data.size() - 1)
            out += QLatin1Char('\n');
    }
    out += QStringLiteral("};\n");
    return out;
}

QString dwordsToC(const QVector<quint32> &data,
                  const QString &name,
                  const DisplayCodeGenerator::CodeGenOptions &opt)
{
    QString out;
    out += arrayDeclLine(QStringLiteral("uint32_t"), name, opt) + QStringLiteral(" = {\n");
    for (int i = 0; i < data.size(); ++i) {
        if (i % 8 == 0)
            out += QStringLiteral("    ");
        out += QStringLiteral("0x%1").arg(data[i], 6, 16, QChar('0'));
        if (i < data.size() - 1)
            out += QStringLiteral(", ");
        if (i % 8 == 7 || i == data.size() - 1)
            out += QLatin1Char('\n');
    }
    out += QStringLiteral("};\n");
    return out;
}

QString asciiArt(const QVector<bool> &bits, int width, int height, bool bricks)
{
    QString out;
    out += QStringLiteral("const char* image_ascii[] = {\n");
    for (int y = 0; y < height; ++y) {
        QString line;
        for (int x = 0; x < width; ++x) {
            const bool on = bits[y * width + x];
            line += on ? (bricks ? QStringLiteral("█") : QStringLiteral("#"))
                       : QStringLiteral(" ");
        }
        out += QStringLiteral("    \"%1\"").arg(line);
        if (y < height - 1)
            out += QStringLiteral(",");
        out += QLatin1Char('\n');
    }
    out += QStringLiteral("};\n");
    return out;
}

} // namespace

QString DisplayCodeGenerator::sanitizeIdentifier(QString name)
{
    name = name.trimmed();
    if (name.isEmpty())
        return QStringLiteral("image_data");
    QString out;
    bool skippedInvalidPrefix = false;
    for (int i = 0; i < name.size(); ++i) {
        const QChar c = name[i];
        if (out.isEmpty()) {
            if (c.isLetter() || c == QLatin1Char('_')) {
                if (skippedInvalidPrefix && c != QLatin1Char('_'))
                    out += QLatin1Char('_');
                out += c;
            } else {
                skippedInvalidPrefix = true;
            }
        } else if (c.isLetterOrNumber() || c == QLatin1Char('_')) {
            out += c;
        }
    }
    return out.isEmpty() ? QStringLiteral("image_data") : out;
}

QVariantList DisplayCodeGenerator::availableEncodings()
{
    struct Entry {
        EncodingMode mode;
        const char *id;
        const char *name;
    };
    static const Entry entries[] = {
        {EncodingMode::Mono1PixPerByte, "mono_1pix", "1 pix/byte"},
        {EncodingMode::Mono8HorizontalLsb, "mono_8h_lsb", "8x Horizontal"},
        {EncodingMode::Mono8HorizontalMsb, "mono_8h_msb", "8x Horizontal MSB"},
        {EncodingMode::Mono8VerticalCol, "mono_8v_col", "8x Vertical Col"},
        {EncodingMode::Mono8VerticalRow, "mono_8v_row", "8x Vertical Row"},
        {EncodingMode::PackedImageAuto, "packed_auto", "Packed Image Auto"},
        {EncodingMode::PackedImageHeader, "packed_header", "Packed Image Header"},
        {EncodingMode::PackedImageRle, "packed_rle", "Packed Image RLE"},
        {EncodingMode::Grayscale8, "grayscale8", "Grayscale"},
        {EncodingMode::Rgb24, "rgb24", "RGB24"},
        {EncodingMode::Rgb888, "rgb888", "RGB888"},
        {EncodingMode::Rgb565, "rgb565", "RGB565"},
        {EncodingMode::Rgb233, "rgb233", "RGB233"},
        {EncodingMode::Ascii, "ascii", "ASCII"},
        {EncodingMode::Bricks, "bricks", "Bricks"}
    };
    QVariantList list;
    for (const Entry &e : entries) {
        QVariantMap m;
        m[QStringLiteral("mode")] = static_cast<int>(e.mode);
        m[QStringLiteral("id")] = QString::fromLatin1(e.id);
        m[QStringLiteral("name")] = QString::fromLatin1(e.name);
        list.append(m);
    }
    return list;
}

QByteArray DisplayCodeGenerator::binaryData(EncodingMode mode,
                                            int width,
                                            int height,
                                            const QVector<bool> &monoBits,
                                            const QByteArray &monoBuffer,
                                            const QByteArray &grayscale8,
                                            const QVector<quint16> &rgb565,
                                            const QByteArray &rgb888,
                                            const QByteArray &rgb233,
                                            const QVector<quint32> &rgb24,
                                            MonoLayout monoLayout)
{
    const int pixels = width * height;
    const bool haveMono = monoBitsReady(monoBits, width, height);

    switch (mode) {
    case EncodingMode::Mono1PixPerByte:
        if (!haveMono)
            return placeholderBinary(mode, width, height, monoLayout);
        return packMono1PixPerByte(monoBits);
    case EncodingMode::Mono8HorizontalLsb:
        if (!haveMono)
            return placeholderBinary(mode, width, height, monoLayout);
        return packMonoHorizontal(monoBits, width, height, false);
    case EncodingMode::Mono8HorizontalMsb:
        if (monoLayout == MonoLayout::Ssd1306Page) {
            const int expected = width * ((height + 7) / 8);
            if (monoBuffer.size() == expected)
                return monoBuffer;
            if (!haveMono)
                return QByteArray(expected, 0);
        } else if (!haveMono) {
            return placeholderBinary(mode, width, height, monoLayout);
        }
        return packMonoHorizontal(monoBits, width, height, true);
    case EncodingMode::Mono8VerticalCol:
        if (!haveMono)
            return placeholderBinary(mode, width, height, monoLayout);
        return packMonoVerticalCol(monoBits, width, height);
    case EncodingMode::Mono8VerticalRow:
        if (!haveMono)
            return placeholderBinary(mode, width, height, monoLayout);
        return packMonoVerticalRow(monoBits, width, height);
    case EncodingMode::PackedImageHeader:
        if (!haveMono)
            return placeholderBinary(mode, width, height, monoLayout);
        return packHeaderBitmap(monoBits, width, height);
    case EncodingMode::PackedImageRle:
        if (!haveMono)
            return placeholderBinary(mode, width, height, monoLayout);
        return packHeaderRle(monoBits, width, height);
    case EncodingMode::PackedImageAuto:
        if (!haveMono)
            return placeholderBinary(mode, width, height, monoLayout);
        return packAutoPackedImage(monoBits, width, height);
    case EncodingMode::Grayscale8:
        if (grayscale8.size() == pixels)
            return grayscale8;
        return QByteArray(pixels, 0);
    case EncodingMode::Rgb888:
        if (rgb888.size() == pixels * 3)
            return rgb888;
        return QByteArray(pixels * 3, 0);
    case EncodingMode::Rgb233:
        if (rgb233.size() == pixels)
            return rgb233;
        return QByteArray(pixels, 0);
    case EncodingMode::Rgb565: {
        if (rgb565.size() != pixels)
            return QByteArray(pixels * 2, 0);
        QByteArray out;
        out.reserve(rgb565.size() * 2);
        for (quint16 v : rgb565) {
            out.append(char(v & 0xFF));
            out.append(char((v >> 8) & 0xFF));
        }
        return out;
    }
    case EncodingMode::Rgb24: {
        if (rgb24.size() != pixels)
            return QByteArray(pixels * 4, 0);
        QByteArray out;
        out.reserve(rgb24.size() * 4);
        for (quint32 v : rgb24) {
            out.append(char(v & 0xFF));
            out.append(char((v >> 8) & 0xFF));
            out.append(char((v >> 16) & 0xFF));
            out.append(char((v >> 24) & 0xFF));
        }
        return out;
    }
    case EncodingMode::Ascii:
    case EncodingMode::Bricks:
        break;
    }
    if (monoLayout == MonoLayout::Ssd1306Page) {
        const int expected = width * ((height + 7) / 8);
        if (monoBuffer.size() == expected)
            return monoBuffer;
        return QByteArray(expected, 0);
    }
    if (!haveMono)
        return placeholderBinary(EncodingMode::Mono8HorizontalMsb, width, height, monoLayout);
    return packMonoHorizontal(monoBits, width, height, true);
}

QString DisplayCodeGenerator::extractArrayBody(const QString &fullCode)
{
    const int idx = fullCode.indexOf(QStringLiteral("const "));
    if (idx < 0)
        return fullCode.trimmed();
    return fullCode.mid(idx).trimmed();
}

QString DisplayCodeGenerator::generate(const DisplayProfile &profile,
                                       int width,
                                       int height,
                                       EncodingMode encodingMode,
                                       const QVector<bool> &monoBits,
                                       const QByteArray &monoBuffer,
                                       const QByteArray &grayscale8,
                                       const QVector<quint16> &rgb565,
                                       const QByteArray &rgb888,
                                       const QByteArray &rgb233,
                                       const QVector<quint32> &rgb24,
                                       const QString &arrayName,
                                       MonoLayout monoLayout)
{
    return generate(profile,
                    width,
                    height,
                    encodingMode,
                    monoBits,
                    monoBuffer,
                    grayscale8,
                    rgb565,
                    rgb888,
                    rgb233,
                    rgb24,
                    arrayName,
                    monoLayout,
                    CodeGenOptions{});
}

QString DisplayCodeGenerator::generate(const DisplayProfile &profile,
                                       int width,
                                       int height,
                                       EncodingMode encodingMode,
                                       const QVector<bool> &monoBits,
                                       const QByteArray &monoBuffer,
                                       const QByteArray &grayscale8,
                                       const QVector<quint16> &rgb565,
                                       const QByteArray &rgb888,
                                       const QByteArray &rgb233,
                                       const QVector<quint32> &rgb24,
                                       const QString &arrayName,
                                       MonoLayout monoLayout,
                                       CodeGenOptions options)
{
    const QString safeName = DisplayCodeGenerator::sanitizeIdentifier(arrayName);
    QString out;
    if (options.includeHeaderComments) {
        out += QStringLiteral("// %1 — %2×%3\n").arg(profile.name).arg(width).arg(height);
        const bool monoMode = encodingMode <= EncodingMode::PackedImageRle
            || encodingMode == EncodingMode::Ascii
            || encodingMode == EncodingMode::Bricks;
        if (monoMode) {
            if (monoLayout == MonoLayout::Ssd1306Page) {
                out += QStringLiteral("// Layout: vertical page buffer (page-major)\n\n");
            } else {
                out += QStringLiteral("// Layout: row-packed (каждая строка массива = строка пикселей)\n\n");
            }
        } else {
            switch (encodingMode) {
            case EncodingMode::Grayscale8:
                out += QStringLiteral("// Layout: row-major grayscale 8-bit\n\n");
                break;
            case EncodingMode::Rgb233:
                out += QStringLiteral("// Layout: row-major RGB233 palette\n\n");
                break;
            case EncodingMode::Rgb888:
                out += QStringLiteral("// Layout: row-major RGB888\n\n");
                break;
            case EncodingMode::Rgb24:
                out += QStringLiteral("// Layout: row-major RGB24\n\n");
                break;
            case EncodingMode::Rgb565:
            default:
                out += QStringLiteral("// Layout: row-major RGB565\n\n");
                break;
            }
        }
    }

    out += QStringLiteral("#define %1_WIDTH  %2\n").arg(safeName.toUpper()).arg(width);
    out += QStringLiteral("#define %1_HEIGHT %2\n").arg(safeName.toUpper()).arg(height);

    if (encodingMode == EncodingMode::Ascii) {
        if (!monoBitsReady(monoBits, width, height))
            return out;
        return out + asciiArt(monoBits, width, height, false);
    }
    if (encodingMode == EncodingMode::Bricks) {
        if (!monoBitsReady(monoBits, width, height))
            return out;
        return out + asciiArt(monoBits, width, height, true);
    }
    if (encodingMode == EncodingMode::Rgb565)
        return out + wordsToC(rgb565, safeName, options);
    if (encodingMode == EncodingMode::Rgb24)
        return out + dwordsToC(rgb24, safeName, options);

    const QByteArray data = binaryData(encodingMode,
                                       width,
                                       height,
                                       monoBits,
                                       monoBuffer,
                                       grayscale8,
                                       rgb565,
                                       rgb888,
                                       rgb233,
                                       rgb24,
                                       monoLayout);
    if (encodingMode == EncodingMode::Rgb888)
        out += byteArrayToC(data, safeName, QStringLiteral("uint8_t"), options);
    else
        out += byteArrayToC(data, safeName, QStringLiteral("uint8_t"), options);
    return out;
}
