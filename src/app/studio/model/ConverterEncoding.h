#ifndef PIXELSTUDIO_APP_CONVERTER_CONVERTERENCODING_H
#define PIXELSTUDIO_APP_CONVERTER_CONVERTERENCODING_H

#include "processing/DisplayCodeGenerator.h"
#include "processing/PixelFormatCatalog.h"

namespace ConverterEncoding {

inline bool isColorMode(int mode)
{
    return !PixelFormatCatalog::isMono(static_cast<DisplayCodeGenerator::EncodingMode>(mode));
}

inline bool isMonoMode(int mode)
{
    return PixelFormatCatalog::isMono(static_cast<DisplayCodeGenerator::EncodingMode>(mode));
}

} // namespace ConverterEncoding

#endif // PIXELSTUDIO_APP_CONVERTER_CONVERTERENCODING_H
