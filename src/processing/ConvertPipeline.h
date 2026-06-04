#ifndef CONVERTPIPELINE_H
#define CONVERTPIPELINE_H

#include <QImage>
#include <QTransform>

#include "processing/DisplayProfile.h"
#include "processing/DisplayRasterizer.h"
#include "processing/ImageFiltersPipeline.h"
#include "processing/DisplayCodeGenerator.h"

struct ConvertPipelineParams
{
    int displayWidth = 128;
    int displayHeight = 64;
    DisplayProfile::ColorMode colorMode = DisplayProfile::Mono1Bit;
    DisplayProfile::ScaleMode scaleMode = DisplayProfile::Fit;
    int monoThreshold = 128;
    bool invertMono = false;
    int rotation = 0;
    bool flipHorizontal = false;
    bool flipVertical = false;
    ImageFiltersPipeline::Params filterParams;
    DisplayCodeGenerator::EncodingMode encodingMode = DisplayCodeGenerator::EncodingMode::Mono8HorizontalMsb;
};

class ConvertPipeline
{
public:
    static QImage applyOrientation(const QImage &source, const ConvertPipelineParams &params);
    static DisplayRasterizer::Result rasterize(const QImage &oriented,
                                               const ConvertPipelineParams &params);
};

#endif // CONVERTPIPELINE_H
