#include "processing/ConvertPipeline.h"

QImage ConvertPipeline::applyOrientation(const QImage &source, const ConvertPipelineParams &params)
{
    if (source.isNull())
        return {};

    QImage transformed = source;
    if (params.flipHorizontal)
        transformed = transformed.mirrored(true, false);
    if (params.flipVertical)
        transformed = transformed.mirrored(false, true);
    if (params.rotation != 0) {
        QTransform t;
        t.rotate(params.rotation);
        transformed = transformed.transformed(t, Qt::SmoothTransformation);
    }
    return transformed;
}

DisplayRasterizer::Result ConvertPipeline::rasterize(const QImage &oriented,
                                                   const ConvertPipelineParams &params)
{
    return DisplayRasterizer::convert(oriented,
                                      params.displayWidth,
                                      params.displayHeight,
                                      params.scaleMode,
                                      params.filterParams,
                                      params.encodingMode,
                                      params.monoThreshold,
                                      params.invertMono,
                                      params.linearColorSpace);
}
