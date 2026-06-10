#include "export/SpriteAtlasService.h"

#include "io/ImageFileReader.h"
#include "translation/AppLocale.h"

#include <QFileInfo>
#include <QPainter>
#include <QtMath>
#include <QVariantMap>

#include "processing/DisplayProfile.h"

namespace {

bool loadImage(const QUrl &url, QImage *out)
{
    QString path = url.toLocalFile();
    if (path.isEmpty())
        path = url.path();
    return ImageFileReader::readFromPath(path, out);
}

QString sanitizedStem(const QUrl &url, int index)
{
    QString path = url.toLocalFile();
    if (path.isEmpty())
        path = url.path();
    return DisplayCodeGenerator::sanitizeIdentifier(QStringLiteral("%1_%2")
        .arg(QFileInfo(path).baseName(), QString::number(index)));
}

} // namespace

SpriteAtlasResult SpriteAtlasService::build(const SpriteAtlasRequest &request)
{
    SpriteAtlasResult out;
    if (request.files.isEmpty()) {
        out.errorMessage = AppLocale::tr("No files for atlas");
        return out;
    }

    QVector<QImage> frames;
    frames.reserve(request.files.size());
    for (const QUrl &url : request.files) {
        QImage img;
        if (!loadImage(url, &img)) {
            out.errorMessage = AppLocale::tr("Failed to load atlas file: %1").arg(url.toString());
            return out;
        }
        frames.append(img);
    }

    const int frameW = request.frameWidth > 0 ? request.frameWidth : request.pipeline.displayWidth;
    const int frameH = request.frameHeight > 0 ? request.frameHeight : request.pipeline.displayHeight;
    const int cols = request.fixedGrid ? qCeil(qSqrt(frames.size())) : frames.size();
    const int rows = request.fixedGrid ? qCeil(qreal(frames.size()) / qreal(cols)) : 1;
    QImage atlas(cols * frameW + qMax(0, cols - 1) * request.padding,
                 rows * frameH + qMax(0, rows - 1) * request.padding,
                 QImage::Format_ARGB32);
    atlas.fill(Qt::transparent);

    QPainter painter(&atlas);
    QString header;
    header += QStringLiteral("// Sprite atlas: %1 frames\n").arg(frames.size());
    header += QStringLiteral("struct PixelStudioSprite { const void* data; uint16_t x; uint16_t y; uint16_t w; uint16_t h; };\n\n");

    QVariantList metadata;
    for (int i = 0; i < frames.size(); ++i) {
        const int col = request.fixedGrid ? (i % cols) : i;
        const int row = request.fixedGrid ? (i / cols) : 0;
        const int x = col * (frameW + request.padding);
        const int y = row * (frameH + request.padding);
        painter.drawImage(x, y, frames[i].scaled(frameW, frameH, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

        ConvertPipelineParams params = request.pipeline;
        params.displayWidth = frameW;
        params.displayHeight = frameH;
        const QImage oriented = ConvertPipeline::applyOrientation(frames[i], params);
        const DisplayRasterizer::Result raster = ConvertPipeline::rasterize(oriented, params);
        DisplayProfile profile;
        profile.id = QStringLiteral("atlas_frame");
        profile.name = QStringLiteral("Atlas frame");
        profile.width = frameW;
        profile.height = frameH;
        profile.colorMode = params.colorMode;
        const QString name = QStringLiteral("%1_%2").arg(request.arrayPrefix, sanitizedStem(request.files[i], i));
        header += DisplayCodeGenerator::generate(profile,
                                                 frameW,
                                                 frameH,
                                                 request.encodingMode,
                                                 raster.monoBits,
                                                 raster.monoBuffer,
                                                 raster.grayscale8,
                                                 raster.rgb565,
                                                 raster.rgb888,
                                                 raster.rgb233,
                                                 raster.rgb24,
                                                 name,
                                                 request.monoLayout,
                                                 request.codeGenOptions,
                                                 raster.indexedPalette);
        header += QLatin1String("\n\n");

        QVariantMap meta;
        meta.insert(QStringLiteral("name"), name);
        meta.insert(QStringLiteral("x"), x);
        meta.insert(QStringLiteral("y"), y);
        meta.insert(QStringLiteral("w"), frameW);
        meta.insert(QStringLiteral("h"), frameH);
        metadata.append(meta);
    }
    painter.end();

    header += QStringLiteral("const PixelStudioSprite %1_sprites[] = {\n").arg(request.arrayPrefix);
    for (const QVariant &entry : metadata) {
        const QVariantMap meta = entry.toMap();
        header += QStringLiteral("  { %1, %2, %3, %4, %5 },\n")
            .arg(meta.value(QStringLiteral("name")).toString())
            .arg(meta.value(QStringLiteral("x")).toInt())
            .arg(meta.value(QStringLiteral("y")).toInt())
            .arg(meta.value(QStringLiteral("w")).toInt())
            .arg(meta.value(QStringLiteral("h")).toInt());
    }
    header += QStringLiteral("};\n");

    out.ok = true;
    out.headerCode = header;
    out.preview = atlas;
    out.metadata = metadata;
    return out;
}
