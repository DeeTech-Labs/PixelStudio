#ifndef PIXELSTUDIO_PROCESSING_PIXELFORMATCATALOG_H
#define PIXELSTUDIO_PROCESSING_PIXELFORMATCATALOG_H

#include <QVariantList>

#include "processing/DisplayCodeGenerator.h"

class PixelFormatCatalog
{
public:
    struct Entry {
        DisplayCodeGenerator::EncodingMode mode;
        const char *id;
        const char *name;
    };

    static constexpr int LegacyEncodingMax = 14;

    static DisplayCodeGenerator::EncodingMode migrateLegacy(int storedValue);
    static bool isMono(DisplayCodeGenerator::EncodingMode mode);
    static bool isGrayscale(DisplayCodeGenerator::EncodingMode mode);
    static bool isColor(DisplayCodeGenerator::EncodingMode mode);

    static const Entry *uiOrder(int *count);
    static QVariantList catalogEntries();
};

#endif // PIXELSTUDIO_PROCESSING_PIXELFORMATCATALOG_H
