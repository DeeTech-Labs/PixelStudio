#include "app/converter/ImagePipelineController.h"
#include "app/DisplayConverter.h"

#include "processing/DisplayCodeGenerator.h"
#include "processing/DisplayProfile.h"
#include "processing/DisplayRasterizer.h"
#include "processing/EncodingAnalyzer.h"

#include <QCoreApplication>
#include <QtConcurrent>

void ImagePipelineController::scheduleRebuild(DisplayConverter &converter, bool immediate)
{
    converter.schedulePersistSession();
    if (immediate) {
        converter.m_rebuildDebounceTimer.stop();
        startAsyncRebuild(converter);
        return;
    }
    converter.m_rebuildDebounceTimer.start();
}

void ImagePipelineController::startAsyncRebuild(DisplayConverter &converter)
{
    if (converter.m_rebuildWatcher.isRunning()) {
        converter.m_rebuildPending = true;
        return;
    }

    if (converter.m_sourceImage.isNull()) {
        converter.m_previewPath.clear();
        converter.m_processPreviewPath.clear();
        converter.m_generatedCode.clear();
        emit converter.previewPathChanged();
        emit converter.processPreviewPathChanged();
        emit converter.generatedCodeChanged();
        return;
    }

    const QImage source = converter.orientedSource();
    const int width = converter.m_displayWidth;
    const int height = converter.m_displayHeight;
    const auto colorMode = converter.m_colorMode;
    const auto scaleMode = converter.m_scaleMode;
    const int monoThreshold = converter.m_monoThreshold;
    const bool invertMono = converter.m_invertMono;
    const ImageFiltersPipeline::Params filterParams = converter.m_filterParams;
    const QString profileId = converter.m_profileId;
    const QString arrayName = converter.m_arrayName;
    const auto encodingMode = converter.m_encodingMode;
    const auto monoLayout = converter.m_monoLayout;
    const DisplayCodeGenerator::CodeGenOptions codeOptions = converter.m_codeGenOptions;
    const bool linearColorSpace = converter.m_linearColorSpace;
    const quint64 generation = ++converter.m_nextGeneration;

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
                                     generation]() -> DisplayConverter::AsyncBuildResult {
        DisplayConverter::AsyncBuildResult output;
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
            profile.name = QCoreApplication::translate("PixelStudio", "Custom %1×%2")
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
    converter.m_rebuildWatcher.setFuture(future);
}

void ImagePipelineController::onAsyncRebuildFinished(DisplayConverter &converter)
{
    const DisplayConverter::AsyncBuildResult result = converter.m_rebuildWatcher.result();
    if (result.generation >= converter.m_lastAppliedGeneration) {
        converter.m_lastAppliedGeneration = result.generation;
        converter.m_lastResult = result.result;

        if (converter.m_lastResult.preview.isNull()) {
            converter.m_previewPath.clear();
            converter.m_processPreviewPath.clear();
            converter.m_generatedCode.clear();
            emit converter.previewPathChanged();
            emit converter.processPreviewPathChanged();
            emit converter.generatedCodeChanged();
            updateCodePreview(converter);
            updateFlashReport(converter);
        } else {
            converter.m_processPreviewPath =
                converter.writeTempPreview(QStringLiteral("process"), converter.m_lastResult.processPreview);
            emit converter.processPreviewPathChanged();
            converter.m_previewPath =
                converter.writeTempPreview(QStringLiteral("preview"), converter.m_lastResult.preview);
            emit converter.previewPathChanged();
            converter.m_generatedCode = result.generatedCode;
            emit converter.generatedCodeChanged();
            updateCodePreview(converter);
            updateFlashReport(converter);
        }
    }

    if (converter.m_rebuildPending) {
        converter.m_rebuildPending = false;
        startAsyncRebuild(converter);
    }
}

void ImagePipelineController::updateCodePreview(DisplayConverter &converter)
{
    if (converter.m_showFullGeneratedCode || converter.m_generatedCode.isEmpty()) {
        converter.m_generatedCodePreview = converter.m_generatedCode;
        converter.m_generatedCodeTruncated = false;
    } else {
        constexpr int kMaxLines = 80;
        constexpr int kMaxChars = 12000;
        QStringList lines = converter.m_generatedCode.split(QLatin1Char('\n'));
        converter.m_generatedCodeTruncated = lines.size() > kMaxLines
            || converter.m_generatedCode.size() > kMaxChars;
        if (lines.size() > kMaxLines)
            lines = lines.mid(0, kMaxLines);
        converter.m_generatedCodePreview = lines.join(QLatin1Char('\n'));
        if (converter.m_generatedCodePreview.size() > kMaxChars)
            converter.m_generatedCodePreview = converter.m_generatedCodePreview.left(kMaxChars);
        if (converter.m_generatedCodeTruncated)
            converter.m_generatedCodePreview += QStringLiteral("\n\n// … %1 bytes omitted — use Copy for full output\n")
                                                     .arg(converter.m_generatedCode.size());
    }
    emit converter.generatedCodePreviewChanged();
}

void ImagePipelineController::updateFlashReport(DisplayConverter &converter)
{
    converter.m_flashReport = EncodingAnalyzer::analyze(converter.m_lastResult,
                                                        converter.m_colorMode,
                                                        converter.m_monoLayout,
                                                        converter.m_encodingMode,
                                                        converter.m_codeGenOptions);
    emit converter.flashReportChanged();
}
