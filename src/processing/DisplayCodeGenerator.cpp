#include "processing/DisplayCodeGenerator.h"
#include "processing/PixelFormatCatalog.h"
#include "processing/PixelFormatPacking.h"

#include <cmath>
#include <QStringList>
#include <QVariantMap>

namespace {

using Mode = DisplayCodeGenerator::EncodingMode;
using MonoLayout = DisplayCodeGenerator::MonoLayout;

QString toHexByte(quint8 value)
{
    return QStringLiteral("0x%1").arg(value, 2, 16, QChar('0'));
}

bool monoBitsReady(const QVector<bool> &bits, int width, int height)
{
    return width > 0 && height > 0 && bits.size() == width * height;
}

QByteArray packMonoHorizontal(const QVector<bool> &bits, int width, int height)
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
                if (bits[y * width + x])
                    byte |= quint8(1u << (7 - bit));
            }
            out[y * bytesPerRow + bx] = char(byte);
        }
    }
    return out;
}

QByteArray packGrayscale4(const QByteArray &grayscale8, int pixels)
{
    if (grayscale8.size() != pixels)
        return QByteArray((pixels + 1) / 2, 0);
    QByteArray out((pixels + 1) / 2, 0);
    for (int i = 0; i < pixels; i += 2) {
        const quint8 hi = quint8((quint8(grayscale8.at(i)) >> 4) & 0x0F);
        const quint8 lo = (i + 1 < pixels)
            ? quint8((quint8(grayscale8.at(i + 1)) >> 4) & 0x0F)
            : quint8(0);
        out[i / 2] = char((hi << 4) | lo);
    }
    return out;
}

QByteArray packBgr888(const QByteArray &rgb888, int pixels)
{
    if (rgb888.size() != pixels * 3)
        return QByteArray(pixels * 3, 0);
    QByteArray out(pixels * 3, 0);
    for (int i = 0; i < pixels; ++i) {
        out[i * 3] = rgb888.at(i * 3 + 2);
        out[i * 3 + 1] = rgb888.at(i * 3 + 1);
        out[i * 3 + 2] = rgb888.at(i * 3);
    }
    return out;
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

QVector<quint16> buildR16f(const QByteArray &grayscale8, int pixels)
{
    QVector<quint16> out(pixels, 0);
    if (grayscale8.size() != pixels)
        return out;
    for (int i = 0; i < pixels; ++i) {
        const float lin = PixelFormatPacking::srgbChannelToLinear(float(quint8(grayscale8.at(i))) / 255.0f);
        out[i] = floatToHalf(lin);
    }
    return out;
}

QVector<float> buildRgba32f(const QByteArray &rgb888, int pixels)
{
    QVector<float> out(pixels * 4, 0.0f);
    if (rgb888.size() != pixels * 3)
        return out;
    for (int i = 0; i < pixels; ++i) {
        out[i * 4] = PixelFormatPacking::srgbChannelToLinear(float(quint8(rgb888.at(i * 3))) / 255.0f);
        out[i * 4 + 1] = PixelFormatPacking::srgbChannelToLinear(float(quint8(rgb888.at(i * 3 + 1))) / 255.0f);
        out[i * 4 + 2] = PixelFormatPacking::srgbChannelToLinear(float(quint8(rgb888.at(i * 3 + 2))) / 255.0f);
        out[i * 4 + 3] = 1.0f;
    }
    return out;
}

int expectedBinarySize(Mode mode, int width, int height, MonoLayout monoLayout)
{
    const int pixels = width * height;
    const int pages = (height + 7) / 8;
    const int bytesPerRow = (width + 7) / 8;
    const int uvStride = (width + 1) & ~1;
    const int chromaRows = (height + 1) / 2;
    const int chromaWidth = (width + 1) / 2;
    switch (mode) {
    case Mode::Mono1Bit:
        if (monoLayout == MonoLayout::Ssd1306Page || monoLayout == MonoLayout::VerticalColumn)
            return width * pages;
        return bytesPerRow * height;
    case Mode::Grayscale4:
        return (pixels + 1) / 2;
    case Mode::Grayscale8:
    case Mode::Indexed8:
        return pixels;
    case Mode::Rgb565:
    case Mode::Bgr565:
        return pixels * 2;
    case Mode::Rgb666:
        return (pixels * 18 + 7) / 8;
    case Mode::Rgb888:
    case Mode::Bgr888:
        return pixels * 3;
    case Mode::Argb8888:
    case Mode::Abgr8888:
        return pixels * 4;
    case Mode::YuvNv12:
        return pixels + uvStride * chromaRows;
    case Mode::YuvYuyv:
        return pixels * 2;
    case Mode::YuvYv12:
        return pixels + chromaWidth * chromaRows * 2;
    case Mode::R16f:
        return pixels * 2;
    case Mode::Rgba32f:
        return pixels * 16;
    case Mode::Count:
        break;
    }
    return 0;
}

QByteArray finalizeBinary(QByteArray data, const DisplayCodeGenerator::CodeGenOptions &options)
{
    return PixelFormatPacking::padForDma(data, options.dmaPaddingAlign);
}

QByteArray placeholderBinary(Mode mode, int width, int height, MonoLayout monoLayout)
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

QString paletteRgb888ToC(const QVector<quint32> &palette,
                         const QString &name,
                         const DisplayCodeGenerator::CodeGenOptions &opt)
{
    QString out;
    out += arrayDeclLine(QStringLiteral("uint8_t"), name, opt);
    out += QStringLiteral("[") + QString::number(palette.size()) + QStringLiteral("][3] = {\n");
    for (int i = 0; i < palette.size(); ++i) {
        out += QStringLiteral("    { 0x%1, 0x%2, 0x%3 }")
                   .arg((palette.at(i) >> 16) & 0xFF, 2, 16, QChar('0'))
                   .arg((palette.at(i) >> 8) & 0xFF, 2, 16, QChar('0'))
                   .arg(palette.at(i) & 0xFF, 2, 16, QChar('0'));
        if (i < palette.size() - 1)
            out += QStringLiteral(",");
        out += QLatin1Char('\n');
    }
    out += QStringLiteral("};\n");
    return out;
}

QString layoutComment(Mode mode, MonoLayout monoLayout)
{
    if (PixelFormatCatalog::isMono(mode)) {
        if (monoLayout == MonoLayout::Ssd1306Page)
            return QStringLiteral("// Layout: vertical page buffer (SSD1306)\n\n");
        if (monoLayout == MonoLayout::VerticalColumn)
            return QStringLiteral("// Layout: vertical column 1-bit (8 px/col byte)\n\n");
        return QStringLiteral("// Layout: row-packed 1-bit (MSB first)\n\n");
    }
    switch (mode) {
    case Mode::Grayscale4:
        return QStringLiteral("// Layout: row-major 4-bit grayscale (2 pixels/byte)\n\n");
    case Mode::Grayscale8:
        return QStringLiteral("// Layout: row-major 8-bit grayscale\n\n");
    case Mode::Indexed8:
        return QStringLiteral("// Layout: row-major 8-bit index + uint8_t palette[][3] LUT (flash = palette + padded indices)\n\n");
    case Mode::Rgb888:
        return QStringLiteral("// Layout: row-major RGB888\n\n");
    case Mode::Argb8888:
        return QStringLiteral("// Layout: row-major ARGB8888 wire bytes B,G,R,A (little-endian)\n\n");
    case Mode::Abgr8888:
        return QStringLiteral("// Layout: row-major ABGR8888 wire bytes R,G,B,A (little-endian)\n\n");
    case Mode::Bgr888:
        return QStringLiteral("// Layout: row-major BGR888\n\n");
    case Mode::Bgr565:
        return QStringLiteral("// Layout: row-major BGR565 wire bytes (uint8_t[2*N], LE unless big-endian SPI)\n\n");
    case Mode::Rgb666:
        return QStringLiteral("// Layout: compact RGB666 (18-bit/pixel, 9 bytes/4 pixels)\n\n");
    case Mode::YuvNv12:
        return QStringLiteral("// Layout: Y plane + interleaved UV (NV12), BT.601 full-range integer\n\n");
    case Mode::YuvYuyv:
        return QStringLiteral("// Layout: packed YUYV 4:2:2, BT.601 full-range integer\n\n");
    case Mode::YuvYv12:
        return QStringLiteral("// Layout: Y + U + V planes (YV12), BT.601 full-range integer\n\n");
    case Mode::R16f:
        return QStringLiteral("// Layout: row-major R16F wire bytes (IEEE754 half LE as uint8_t[2*N])\n\n");
    case Mode::Rgba32f:
        return QStringLiteral("// Layout: row-major RGBA32F wire bytes (IEEE754 float LE as uint8_t[16*N])\n\n");
    case Mode::Rgb565:
        return QStringLiteral("// Layout: row-major RGB565 wire bytes (uint8_t[2*N], LE unless big-endian SPI)\n\n");
    default:
        return QStringLiteral("// Layout: row-major pixel buffer\n\n");
    }
}

QByteArray packArgb8888Le(const QVector<quint32> &rgba, int pixels, bool abgr)
{
    QByteArray out(pixels * 4, 0);
    for (int i = 0; i < pixels && i < rgba.size(); ++i) {
        const quint32 v = rgba[i];
        const quint8 a = quint8((v >> 24) & 0xFF);
        const quint8 r = quint8((v >> 16) & 0xFF);
        const quint8 g = quint8((v >> 8) & 0xFF);
        const quint8 b = quint8(v & 0xFF);
        if (abgr) {
            out[i * 4] = char(r);
            out[i * 4 + 1] = char(g);
            out[i * 4 + 2] = char(b);
            out[i * 4 + 3] = char(a);
        } else {
            out[i * 4] = char(b);
            out[i * 4 + 1] = char(g);
            out[i * 4 + 2] = char(r);
            out[i * 4 + 3] = char(a);
        }
    }
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
    return PixelFormatCatalog::catalogEntries();
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
                                            MonoLayout monoLayout,
                                            const CodeGenOptions &options)
{
    const int pixels = width * height;
    const bool haveMono = monoBitsReady(monoBits, width, height);
    QByteArray data;

    switch (mode) {
    case Mode::Mono1Bit:
        if (monoLayout == MonoLayout::Ssd1306Page) {
            const int expected = width * ((height + 7) / 8);
            data = monoBuffer.size() == expected ? monoBuffer : QByteArray(expected, 0);
        } else if (monoLayout == MonoLayout::VerticalColumn) {
            if (!haveMono)
                data = placeholderBinary(mode, width, height, monoLayout);
            else
                data = PixelFormatPacking::packMonoVerticalCol(monoBits, width, height);
        } else if (!haveMono) {
            data = placeholderBinary(mode, width, height, monoLayout);
        } else {
            data = packMonoHorizontal(monoBits, width, height);
        }
        break;
    case Mode::Grayscale4:
        data = packGrayscale4(grayscale8, pixels);
        break;
    case Mode::Grayscale8:
        data = grayscale8.size() == pixels ? grayscale8 : QByteArray(pixels, 0);
        break;
    case Mode::Indexed8:
        data = rgb233.size() == pixels ? rgb233 : QByteArray(pixels, 0);
        break;
    case Mode::Rgb565:
        if (rgb565.size() != pixels)
            data = QByteArray(pixels * 2, 0);
        else
            data = PixelFormatPacking::packRgb565Stream(rgb565, pixels, options.rgb565BigEndian);
        break;
    case Mode::Bgr565: {
        if (rgb565.size() != pixels) {
            data = QByteArray(pixels * 2, 0);
            break;
        }
        QVector<quint16> swapped(rgb565.size());
        for (int i = 0; i < rgb565.size(); ++i)
            swapped[i] = PixelFormatPacking::swapRgb565ToBgr565(rgb565.at(i));
        data = PixelFormatPacking::packRgb565Stream(swapped, pixels, options.rgb565BigEndian);
        break;
    }
    case Mode::Rgb666:
        data = PixelFormatPacking::packRgb666Compact(rgb888, pixels);
        break;
    case Mode::Rgb888:
        data = rgb888.size() == pixels * 3 ? rgb888 : QByteArray(pixels * 3, 0);
        break;
    case Mode::Bgr888:
        data = packBgr888(rgb888, pixels);
        break;
    case Mode::Argb8888:
        data = rgb24.size() != pixels ? QByteArray(pixels * 4, 0) : packArgb8888Le(rgb24, pixels, false);
        break;
    case Mode::Abgr8888:
        data = rgb24.size() != pixels ? QByteArray(pixels * 4, 0) : packArgb8888Le(rgb24, pixels, true);
        break;
    case Mode::YuvYuyv:
        data = PixelFormatPacking::packYuvYuyv(rgb888, width, height);
        break;
    case Mode::YuvNv12:
        data = PixelFormatPacking::packYuvNv12(rgb888, width, height);
        break;
    case Mode::YuvYv12:
        data = PixelFormatPacking::packYuvYv12(rgb888, width, height);
        break;
    case Mode::R16f:
        data = PixelFormatPacking::packHalfLe(buildR16f(grayscale8, pixels));
        break;
    case Mode::Rgba32f:
        data = PixelFormatPacking::packFloatLe(buildRgba32f(rgb888, pixels));
        break;
    case Mode::Count:
        break;
    }
    return finalizeBinary(data, options);
}

int DisplayCodeGenerator::flashFootprintBytes(EncodingMode mode,
                                              int width,
                                              int height,
                                              const QVector<bool> &monoBits,
                                              const QByteArray &monoBuffer,
                                              const QByteArray &grayscale8,
                                              const QVector<quint16> &rgb565,
                                              const QByteArray &rgb888,
                                              const QByteArray &rgb233,
                                              const QVector<quint32> &rgb24,
                                              MonoLayout monoLayout,
                                              const CodeGenOptions &options,
                                              const QVector<quint32> &indexedPalette)
{
    int bytes = binaryData(mode,
                           width,
                           height,
                           monoBits,
                           monoBuffer,
                           grayscale8,
                           rgb565,
                           rgb888,
                           rgb233,
                           rgb24,
                           monoLayout,
                           options)
                .size();
    if (mode == Mode::Indexed8 && !indexedPalette.isEmpty())
        bytes += indexedPalette.size() * 3;
    return bytes;
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
                                       MonoLayout monoLayout,
                                       const QVector<quint32> &indexedPalette)
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
                    CodeGenOptions{},
                    indexedPalette);
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
                                       CodeGenOptions options,
                                       const QVector<quint32> &indexedPalette)
{
    const QString safeName = DisplayCodeGenerator::sanitizeIdentifier(arrayName);
    QString out;
    if (options.includeHeaderComments) {
        out += QStringLiteral("// %1 — %2×%3\n").arg(profile.name).arg(width).arg(height);
        out += layoutComment(encodingMode, monoLayout);
    }

    out += QStringLiteral("#define %1_WIDTH  %2\n").arg(safeName.toUpper()).arg(width);
    out += QStringLiteral("#define %1_HEIGHT %2\n").arg(safeName.toUpper()).arg(height);

    if (encodingMode == Mode::Rgb565 || encodingMode == Mode::Bgr565) {
        if (rgb565.isEmpty())
            return out;
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
                                           monoLayout,
                                           options);
        return out + byteArrayToC(data, safeName, QStringLiteral("uint8_t"), options);
    }
    if (encodingMode == Mode::Indexed8) {
        const int pixels = width * height;
        if (rgb233.size() != pixels || indexedPalette.isEmpty())
            return out;
        const QString paletteName = safeName + QStringLiteral("_palette");
        out += QStringLiteral("#define %1_PALETTE_SIZE %2\n")
                   .arg(safeName.toUpper())
                   .arg(indexedPalette.size());
        out += paletteRgb888ToC(indexedPalette, paletteName, options);
        const QByteArray indices = binaryData(encodingMode,
                                              width,
                                              height,
                                              monoBits,
                                              monoBuffer,
                                              grayscale8,
                                              rgb565,
                                              rgb888,
                                              rgb233,
                                              rgb24,
                                              monoLayout,
                                              options);
        return out + byteArrayToC(indices, safeName, QStringLiteral("uint8_t"), options);
    }
    if (encodingMode == Mode::Argb8888 || encodingMode == Mode::Abgr8888) {
        if (rgb24.isEmpty())
            return out;
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
                                           monoLayout,
                                           options);
        return out + byteArrayToC(data, safeName, QStringLiteral("uint8_t"), options);
    }
    if (encodingMode == Mode::R16f) {
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
                                           monoLayout,
                                           options);
        if (data.isEmpty())
            return out;
        return out + byteArrayToC(data, safeName, QStringLiteral("uint8_t"), options);
    }
    if (encodingMode == Mode::Rgba32f) {
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
                                           monoLayout,
                                           options);
        if (data.isEmpty())
            return out;
        return out + byteArrayToC(data, safeName, QStringLiteral("uint8_t"), options);
    }

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
                                       monoLayout,
                                       options);
    return out + byteArrayToC(data, safeName, QStringLiteral("uint8_t"), options);
}
