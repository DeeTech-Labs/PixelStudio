#ifndef PIXELSTUDIO_APP_CONVERTER_CONVERTERTABSNAPSHOT_H
#define PIXELSTUDIO_APP_CONVERTER_CONVERTERTABSNAPSHOT_H

#include <QImage>
#include <QUrl>

#include "app/studio/model/ConverterState.h"
#include "persistence/ProjectService.h"

struct ConverterTabSnapshot
{
    QString tabId;
    StudioProject project;
    QImage sourceImage;
    QString sourceFilePath;
    QUrl projectFileUrl;
    DisplayRasterizer::Result lastResult;
    QString generatedCode;
    bool hasPipelineResult = false;
    QUrl sourcePath;
    QUrl previewPath;
    QUrl processPreviewPath;
};

#endif // PIXELSTUDIO_APP_CONVERTER_CONVERTERTABSNAPSHOT_H
