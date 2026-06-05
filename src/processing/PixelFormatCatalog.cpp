#include "processing/PixelFormatCatalog.h"

#include <QVariantMap>

namespace {

using Mode = DisplayCodeGenerator::EncodingMode;

constexpr PixelFormatCatalog::Entry kUiOrder[] = {
    {Mode::Rgb565, "rgb565", "RGB565 (16-bit)"},
    {Mode::Rgb888, "rgb888", "RGB888 (24-bit)"},
    {Mode::Argb8888, "argb8888", "ARGB8888 (32-bit)"},
    {Mode::Mono1Bit, "mono_1bit", "Monochrome (1-bit)"},
    {Mode::Grayscale4, "grayscale4", "Grayscale (4-bit)"},
    {Mode::Grayscale8, "grayscale8", "Grayscale (8-bit)"},
    {Mode::Indexed8, "indexed8", "Indexed color (8-bit palette)"},
    {Mode::Rgb666, "rgb666", "RGB666 (18-bit)"},
    {Mode::Bgr565, "bgr565", "BGR565 (16-bit)"},
    {Mode::Bgr888, "bgr888", "BGR888 (24-bit)"},
    {Mode::Abgr8888, "abgr8888", "ABGR8888 (32-bit)"},
    {Mode::YuvNv12, "yuv_nv12", "YUV NV12"},
    {Mode::YuvYuyv, "yuv_yuyv", "YUV YUYV"},
    {Mode::YuvYv12, "yuv_yv12", "YUV YV12"},
    {Mode::R16f, "r16f", "R16F (16-bit float)"},
    {Mode::Rgba32f, "rgba32f", "RGBA32F (32-bit float)"},
};

Mode mapLegacy(int value)
{
    switch (value) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 13:
    case 14:
        return Mode::Mono1Bit;
    case 8:
        return Mode::Grayscale8;
    case 9:
        return Mode::Argb8888;
    case 10:
        return Mode::Rgb888;
    case 11:
        return Mode::Rgb565;
    case 12:
        return Mode::Indexed8;
    default:
        break;
    }
    if (value >= 0 && value < static_cast<int>(Mode::Count))
        return static_cast<Mode>(value);
    return Mode::Mono1Bit;
}

} // namespace

DisplayCodeGenerator::EncodingMode PixelFormatCatalog::migrateLegacy(int storedValue)
{
    if (storedValue >= 0 && storedValue <= LegacyEncodingMax)
        return mapLegacy(storedValue);
    if (storedValue >= 0 && storedValue < static_cast<int>(Mode::Count))
        return static_cast<Mode>(storedValue);
    return Mode::Mono1Bit;
}

bool PixelFormatCatalog::isMono(DisplayCodeGenerator::EncodingMode mode)
{
    return mode == Mode::Mono1Bit;
}

bool PixelFormatCatalog::isGrayscale(DisplayCodeGenerator::EncodingMode mode)
{
    return mode == Mode::Grayscale4 || mode == Mode::Grayscale8;
}

bool PixelFormatCatalog::isColor(DisplayCodeGenerator::EncodingMode mode)
{
    return !isMono(mode) && !isGrayscale(mode);
}

const PixelFormatCatalog::Entry *PixelFormatCatalog::uiOrder(int *count)
{
    if (count)
        *count = int(sizeof(kUiOrder) / sizeof(kUiOrder[0]));
    return kUiOrder;
}

QVariantList PixelFormatCatalog::catalogEntries()
{
    int count = 0;
    const Entry *order = uiOrder(&count);
    QVariantList list;
    list.reserve(count);
    for (int i = 0; i < count; ++i) {
        QVariantMap m;
        m[QStringLiteral("mode")] = static_cast<int>(order[i].mode);
        m[QStringLiteral("id")] = QString::fromLatin1(order[i].id);
        m[QStringLiteral("name")] = QString::fromLatin1(order[i].name);
        list.append(m);
    }
    return list;
}
