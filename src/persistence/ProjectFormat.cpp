#include "persistence/ProjectFormat.h"

#include "persistence/ProjectFormatDecoder.h"
#include "persistence/ProjectFormatEncoder.h"
#include "persistence/ProjectService.h"
#include "persistence/RecentPreview.h"
#include "processing/DisplayCodeGenerator.h"
#include "processing/DisplayRasterizer.h"
#include "processing/DisplayProfile.h"
#include "processing/PixelFormatCatalog.h"

#include <QBuffer>
#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QImageReader>

namespace {

bool isRasterImagePath(const QString &path)
{
    static const QStringList exts = {
        QStringLiteral("png"),
        QStringLiteral("jpg"),
        QStringLiteral("jpeg"),
        QStringLiteral("bmp"),
        QStringLiteral("gif"),
        QStringLiteral("webp"),
    };
    return exts.contains(QFileInfo(path).suffix(), Qt::CaseInsensitive);
}

QString encodingLabelForMode(int encodingMode)
{
    using Mode = DisplayCodeGenerator::EncodingMode;
    const auto mode = PixelFormatCatalog::migrateLegacy(encodingMode);
    int count = 0;
    const PixelFormatCatalog::Entry *order = PixelFormatCatalog::uiOrder(&count);
    for (int i = 0; i < count; ++i) {
        if (order[i].mode == mode)
            return QCoreApplication::translate("PixelStudio", order[i].name);
    }
    if (PixelFormatCatalog::isMono(mode))
        return QCoreApplication::translate("PixelStudio", "Monochrome (1-bit)");
    if (PixelFormatCatalog::isGrayscale(mode))
        return QCoreApplication::translate("PixelStudio", "Grayscale (8-bit)");
    return QCoreApplication::translate("PixelStudio", "Color");
}

QString specsMeta(int width, int height, int encodingMode)
{
    return QStringLiteral("%1x%2px • %3")
        .arg(width)
        .arg(height)
        .arg(encodingLabelForMode(encodingMode));
}

DisplayProfile::ColorMode colorModeFromEncoding(int encodingMode)
{
    const auto mode = PixelFormatCatalog::migrateLegacy(encodingMode);
    return PixelFormatCatalog::isMono(mode) ? DisplayProfile::Mono1Bit : DisplayProfile::Rgb565;
}

bool loadSourceImage(const StudioProject &project, QImage *image)
{
    if (!image)
        return false;
    if (!project.sourceImagePng.isEmpty()) {
        if (image->loadFromData(project.sourceImagePng, "PNG"))
            return true;
    }
    for (const ProjectAsset &asset : project.assets) {
        if (!isRasterImagePath(asset.path) || !QFileInfo::exists(asset.path))
            continue;
        QImageReader reader(asset.path);
        if (reader.read(image))
            return true;
    }
    return false;
}

QImage resultPreviewForProject(const StudioProject &project)
{
    if (!project.resultPreviewPng.isEmpty()) {
        QImage image;
        if (image.loadFromData(project.resultPreviewPng, "PNG"))
            return image;
    }

    QImage source;
    if (!loadSourceImage(project, &source))
        return {};

    const SessionSnapshot &s = project.session;
    const auto encodingMode = static_cast<DisplayCodeGenerator::EncodingMode>(
        PixelFormatCatalog::migrateLegacy(s.encodingMode));
    const auto scaleMode = static_cast<DisplayProfile::ScaleMode>(qBound(0, s.scaleMode, 2));
    const DisplayRasterizer::Result result = DisplayRasterizer::convert(
        source,
        s.displayWidth,
        s.displayHeight,
        colorModeFromEncoding(s.encodingMode),
        scaleMode,
        s.filterParams,
        encodingMode,
        s.monoThreshold,
        s.invertMono,
        s.linearColorSpace);
    return result.preview;
}

} // namespace

QString ProjectFormat::extension()
{
    return QStringLiteral(".pspx");
}

bool ProjectFormat::isNativePayload(const QByteArray &head)
{
    return head.size() >= 4
        && head.at(0) == kMagic[0]
        && head.at(1) == kMagic[1]
        && head.at(2) == kMagic[2]
        && head.at(3) == kMagic[3];
}

bool ProjectFormat::isProjectPath(const QString &path)
{
    return path.endsWith(extension(), Qt::CaseInsensitive);
}

QByteArray ProjectFormat::encode(const StudioProject &project)
{
    return ProjectFormatEncoder::encodeFile(project);
}

bool ProjectFormat::decode(const QByteArray &fileData, StudioProject *project, QString *errorText)
{
    return ProjectFormatDecoder::decodeFile(fileData, project, errorText);
}

bool ProjectFormat::loadRecentSummary(const QString &path, RecentSummary *summary, QString *errorText)
{
    if (!summary)
        return false;
    *summary = {};

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        if (errorText)
            *errorText = QStringLiteral("Cannot open project");
        return false;
    }

    StudioProject project;
    if (!decode(file.readAll(), &project, errorText))
        return false;

    summary->title = project.name;
    summary->hasImageContent = ProjectService::hasImageContent(project);
    summary->resultWidth = project.session.displayWidth;
    summary->resultHeight = project.session.displayHeight;
    summary->encodingLabel = encodingLabelForMode(project.session.encodingMode);
    summary->specsMeta = specsMeta(summary->resultWidth,
                                   summary->resultHeight,
                                   project.session.encodingMode);

    QByteArray previewPng = project.resultPreviewPng;
    if (previewPng.isEmpty()) {
        const QImage preview = resultPreviewForProject(project);
        if (!preview.isNull()) {
            QBuffer buffer(&previewPng);
            buffer.open(QIODevice::WriteOnly);
            preview.save(&buffer, "PNG");
        }
    }
    if (!previewPng.isEmpty())
        summary->thumbnailPath = RecentPreview::resolveThumbnail(path, previewPng);

    return true;
}
