#ifndef PIXELSTUDIO_PROCESSING_ENCODINGANALYZER_H
#define PIXELSTUDIO_PROCESSING_ENCODINGANALYZER_H

#include <QVariantList>

#include "processing/DisplayCodeGenerator.h"
#include "processing/DisplayProfile.h"
#include "processing/DisplayRasterizer.h"

class EncodingAnalyzer
{
public:
    static QVariantList analyze(const DisplayRasterizer::Result &result,
                                DisplayProfile::ColorMode colorMode,
                                DisplayCodeGenerator::MonoLayout monoLayout,
                                DisplayCodeGenerator::EncodingMode currentMode,
                                const DisplayCodeGenerator::CodeGenOptions &options = DisplayCodeGenerator::CodeGenOptions{});
};

#endif // PIXELSTUDIO_PROCESSING_ENCODINGANALYZER_H
