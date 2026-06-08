#ifndef PIXELSTUDIO_APP_CONVERTER_CONVERTERTABSNAPSHOT_H
#define PIXELSTUDIO_APP_CONVERTER_CONVERTERTABSNAPSHOT_H

#include <QImage>
#include <QUrl>

#include "app/converter/ConverterState.h"
#include "persistence/ProjectService.h"

struct ConverterTabSnapshot
{
    StudioProject project;
    QImage sourceImage;
    QString sourceFilePath;
    QUrl projectFileUrl;
    DisplayRasterizer::Result lastResult;
    QString generatedCode;
    bool hasPipelineResult = false;
};

#endif // PIXELSTUDIO_APP_CONVERTER_CONVERTERTABSNAPSHOT_H
