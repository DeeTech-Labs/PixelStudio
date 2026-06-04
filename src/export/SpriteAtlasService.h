#ifndef PIXELSTUDIO_EXPORT_SPRITEATLASSERVICE_H
#define PIXELSTUDIO_EXPORT_SPRITEATLASSERVICE_H

#include <QImage>
#include <QUrl>
#include <QVector>
#include <QVariantList>

#include "processing/ConvertPipeline.h"
#include "processing/DisplayCodeGenerator.h"

struct SpriteAtlasRequest
{
    QVector<QUrl> files;
    ConvertPipelineParams pipeline;
    QString arrayPrefix = QStringLiteral("sprite");
    int frameWidth = 0;
    int frameHeight = 0;
    int padding = 0;
    bool fixedGrid = false;
    DisplayCodeGenerator::EncodingMode encodingMode = DisplayCodeGenerator::EncodingMode::Mono8HorizontalMsb;
    DisplayCodeGenerator::MonoLayout monoLayout = DisplayCodeGenerator::MonoLayout::RowPacked;
};

struct SpriteAtlasResult
{
    bool ok = false;
    QString errorMessage;
    QString headerCode;
    QImage preview;
    QVariantList metadata;
};

class SpriteAtlasService
{
public:
    static SpriteAtlasResult build(const SpriteAtlasRequest &request);
};

#endif // PIXELSTUDIO_EXPORT_SPRITEATLASSERVICE_H
