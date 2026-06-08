#ifndef PIXELSTUDIO_APP_CONVERTER_CONVERTERSTATE_H
#define PIXELSTUDIO_APP_CONVERTER_CONVERTERSTATE_H

#include <QImage>
#include <QUrl>
#include <QString>
#include <QVariantList>

#include "processing/DisplayProfile.h"
#include "processing/DisplayRasterizer.h"
#include "processing/DisplayCodeGenerator.h"
#include "processing/ImageFiltersPipeline.h"
#include "persistence/SessionSettings.h"
#include "persistence/ProjectService.h"

struct ConverterAsyncBuildResult
{
    quint64 generation = 0;
    DisplayRasterizer::Result result;
    QString generatedCode;
};

struct ConverterState
{
    QImage sourceImage;
    QUrl sourcePath;
    QUrl previewPath;
    QUrl processPreviewPath;
    quint64 previewEpoch = 0;
    QString generatedCode;
    QString generatedCodePreview;
    bool generatedCodeTruncated = false;
    bool showFullGeneratedCode = false;
    DisplayRasterizer::Result lastResult;

    QString profileId = QStringLiteral("128x64");
    int displayWidth = 128;
    int displayHeight = 64;
    DisplayProfile::ColorMode colorMode = DisplayProfile::Mono1Bit;
    DisplayProfile::ScaleMode scaleMode = DisplayProfile::ScaleMode::Fit;
    bool dithering = true;
    int monoThreshold = 128;
    QString arrayName = QStringLiteral("image_data");
    DisplayCodeGenerator::MonoLayout monoLayout = DisplayCodeGenerator::MonoLayout::RowPacked;
    DisplayCodeGenerator::EncodingMode encodingMode = DisplayCodeGenerator::EncodingMode::Mono1Bit;
    int rotation = 0;
    bool flipHorizontal = false;
    bool flipVertical = false;
    bool invertMono = false;
    ImageFiltersPipeline::Params filterParams;
    DisplayCodeGenerator::CodeGenOptions codeGenOptions;
    bool linearColorSpace = true;
    mutable QImage orientedCache;
    mutable bool orientedDirty = true;

    quint64 nextGeneration = 0;
    quint64 lastAppliedGeneration = 0;
    bool rebuildPending = false;
    bool showGrid = false;
    int gridThresholdZoom = 8;
    StudioProject project;
    QString sourceFilePath;
    QUrl projectFile;
    int offsetX = 0;
    int offsetY = 0;
    QVariantList flashReport;
    SessionUiState uiState;
    int localizationRevision = 0;
    bool imageLoading = false;
};

#endif // PIXELSTUDIO_APP_CONVERTER_CONVERTERSTATE_H
