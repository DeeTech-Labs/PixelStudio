#include "app/converter/ImagePipelineController.h"
#include "app/converter/ConverterState.h"
#include "app/DisplayConverter.h"

#include "processing/DisplayCodeGenerator.h"
#include "processing/DisplayProfile.h"
#include "processing/DisplayRasterizer.h"
#include "processing/EncodingAnalyzer.h"
#include "translation/AppLocale.h"
#include <QtConcurrent>

void ImagePipelineController::scheduleRebuild(DisplayConverter &converter, bool immediate)
{
    converter.schedulePersistSession();
    if (immediate) {
        converter.rebuildDebounceTimer()->stop();
        startAsyncRebuild(converter);
        return;
    }
    converter.rebuildDebounceTimer()->start();
}

void ImagePipelineController::startAsyncRebuild(DisplayConverter &converter)
{
    ConverterState &state = converter.converterState();
    if (converter.rebuildWatcher()->isRunning()) {
        state.rebuildPending = true;
        return;
    }

    if (state.sourceImage.isNull()) {
        state.previewPath.clear();
        state.processPreviewPath.clear();
        state.generatedCode.clear();
        emit converter.previewPathChanged();
        emit converter.processPreviewPathChanged();
        emit converter.generatedCodeChanged();
        return;
    }

    const QImage source = converter.orientedSource();
    const int width = state.displayWidth;
    const int height = state.displayHeight;
    const auto colorMode = state.colorMode;
    const auto scaleMode = state.scaleMode;
    const int monoThreshold = state.monoThreshold;
    const bool invertMono = state.invertMono;
    const ImageFiltersPipeline::Params filterParams = state.filterParams;
    const QString profileId = state.profileId;
    const QString arrayName = state.arrayName;
    const auto encodingMode = state.encodingMode;
    const auto monoLayout = state.monoLayout;
    const DisplayCodeGenerator::CodeGenOptions codeOptions = state.codeGenOptions;
    const bool linearColorSpace = state.linearColorSpace;
    const quint64 generation = ++state.nextGeneration;

    auto future = QtConcurrent::run([source,
                                     width,
                                     height,
                                     colorMode,
                                     scaleMode,
                                     monoThreshold,
                                     invertMono,
                                     filterParams,
                                     profileId,
                                     arrayName,
                                     encodingMode,
                                     monoLayout,
                                     codeOptions,
                                     linearColorSpace,
                                     generation]() -> ConverterAsyncBuildResult {
        ConverterAsyncBuildResult output;
        output.generation = generation;
        output.result = DisplayRasterizer::convert(source,
                                                   width,
                                                   height,
                                                   colorMode,
                                                   scaleMode,
                                                   filterParams,
                                                   encodingMode,
                                                   monoThreshold,
                                                   invertMono,
                                                   linearColorSpace);
        if (output.result.preview.isNull())
            return output;

        DisplayProfile profile = DisplayProfile::byId(profileId);
        profile.width = width;
        profile.height = height;
        profile.colorMode = colorMode;
        if (profile.id == QStringLiteral("custom"))
            profile.name = AppLocale::tr("Custom %1×%2")
                               .arg(width)
                               .arg(height);

        output.generatedCode = DisplayCodeGenerator::generate(profile,
                                                              width,
                                                              height,
                                                              encodingMode,
                                                              output.result.monoBits,
                                                              output.result.monoBuffer,
                                                              output.result.grayscale8,
                                                              output.result.rgb565,
                                                              output.result.rgb888,
                                                              output.result.rgb233,
                                                              output.result.rgb24,
                                                              arrayName,
                                                              monoLayout,
                                                              codeOptions,
                                                              output.result.indexedPalette);
        return output;
    });
    converter.rebuildWatcher()->setFuture(future);
}

void ImagePipelineController::onAsyncRebuildFinished(DisplayConverter &converter)
{
    ConverterState &state = converter.converterState();
    const ConverterAsyncBuildResult result = converter.rebuildWatcher()->result();
    if (result.generation >= state.lastAppliedGeneration) {
        converter.applyPipelineResult(result);
    }

    if (state.rebuildPending) {
        state.rebuildPending = false;
        startAsyncRebuild(converter);
    }
}

void ImagePipelineController::updateCodePreview(ConverterState &state, DisplayConverter &converter)
{
    if (state.showFullGeneratedCode || state.generatedCode.isEmpty()) {
        state.generatedCodePreview = state.generatedCode;
        state.generatedCodeTruncated = false;
    } else {
        constexpr int kMaxLines = 80;
        constexpr int kMaxChars = 12000;
        QStringList lines = state.generatedCode.split(QLatin1Char('\n'));
        state.generatedCodeTruncated = lines.size() > kMaxLines
            || state.generatedCode.size() > kMaxChars;
        if (lines.size() > kMaxLines)
            lines = lines.mid(0, kMaxLines);
        state.generatedCodePreview = lines.join(QLatin1Char('\n'));
        if (state.generatedCodePreview.size() > kMaxChars)
            state.generatedCodePreview = state.generatedCodePreview.left(kMaxChars);
        if (state.generatedCodeTruncated)
            state.generatedCodePreview += QStringLiteral("\n\n// … %1 bytes omitted — use Copy for full output\n")
                                                     .arg(state.generatedCode.size());
    }
    emit converter.generatedCodePreviewChanged();
}

void ImagePipelineController::updateFlashReport(ConverterState &state, DisplayConverter &converter)
{
    state.flashReport = EncodingAnalyzer::analyze(state.lastResult,
                                                  state.colorMode,
                                                  state.monoLayout,
                                                  state.encodingMode,
                                                  state.codeGenOptions);
    emit converter.flashReportChanged();
}
