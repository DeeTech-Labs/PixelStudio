#include "persistence/ProjectFormatDecoder.h"

#include "LogCategories.h"
#include "persistence/ProjectFormat.h"
#include "persistence/ProjectFormatIo.h"
#include "persistence/ProjectService.h"
#include "persistence/StoredPath.h"
#include "processing/DisplayCodeGenerator.h"
#include "processing/ImageFiltersPipeline.h"
#include "processing/PixelFormatCatalog.h"

#include <QColor>
#include <QFileInfo>

namespace {

using namespace ProjectFormatIo;

void applySessionFlags(quint32 f, SessionSnapshot *session)
{
    session->dithering = (f >> 0) & 1;
    session->flipHorizontal = (f >> 1) & 1;
    session->flipVertical = (f >> 2) & 1;
    session->invertMono = (f >> 3) & 1;
    session->showGrid = (f >> 4) & 1;
    session->codeIncludeComments = (f >> 5) & 1;
    session->codeUseProgmem = (f >> 6) & 1;
    session->codeStaticStorage = (f >> 7) & 1;
    session->rgb565BigEndian = (f >> 8) & 1;
    session->linearColorSpace = (f >> 9) & 1;
    const int dma = (f >> 10) & 0x3;
    session->codeDmaAlign = dma == 2 ? 8 : (dma == 1 ? 4 : 0);
}

void applyFilterFlags(quint32 f, ImageFiltersPipeline::Params *p)
{
    p->blackBackground = (f >> 0) & 1;
    p->colorMaskEnabled = (f >> 1) & 1;
    p->sharpen = (f >> 2) & 1;
    p->thresholdEnabled = (f >> 3) & 1;
    p->invert = (f >> 4) & 1;
}

int decodeProjectEncoding(quint8 stored, quint8 formatVersion)
{
    using Mode = DisplayCodeGenerator::EncodingMode;
    const int nativeMax = static_cast<int>(Mode::Count);
    if (formatVersion >= 2) {
        if (stored < static_cast<quint8>(nativeMax))
            return static_cast<int>(stored);
        return static_cast<int>(Mode::Mono1Bit);
    }
    return static_cast<int>(PixelFormatCatalog::migrateLegacy(stored));
}

bool readSessionBlock(const QByteArray &payload,
                      int &pos,
                      quint8 formatVersion,
                      SessionSnapshot *session,
                      QString *errorText)
{
    quint16 sessionBits = 0;
    quint8 u8 = 0;
    quint16 u16 = 0;
    if (!readU16(payload, pos, &sessionBits)
        || !readU8(payload, pos, &u8)) {
        if (errorText)
            *errorText = QStringLiteral("Invalid session header");
        return false;
    }
    applySessionFlags(sessionBits, session);
    session->scaleMode = u8;

    if (!readU8(payload, pos, &u8))
        return false;
    session->monoLayout = u8;
    if (!readU8(payload, pos, &u8))
        return false;
    session->rotation = u8;
    if (!readU8(payload, pos, &u8))
        return false;
    session->encodingMode = decodeProjectEncoding(u8, formatVersion);
    if (!readU8(payload, pos, &u8))
        return false;
    session->gridThresholdZoom = u8;
    if (!readU16(payload, pos, &u16))
        return false;
    session->displayWidth = u16;
    if (!readU16(payload, pos, &u16))
        return false;
    session->displayHeight = u16;
    if (!readU16(payload, pos, &u16))
        return false;
    session->monoThreshold = u16;
    if (!readString(payload, pos, &session->profileId)
        || !readString(payload, pos, &session->arrayName)) {
        if (errorText)
            *errorText = QStringLiteral("Invalid session profile");
        return false;
    }

    ImageFiltersPipeline::Params filters;
    quint16 filterBits = 0;
    if (!readU16(payload, pos, &filterBits))
        return false;
    applyFilterFlags(filterBits, &filters);

    auto readFilterU8 = [&](int *target) {
        if (!readU8(payload, pos, &u8))
            return false;
        *target = u8;
        return true;
    };
    if (!readFilterU8(&filters.brightness)
        || !readFilterU8(&filters.contrast)
        || !readFilterU8(&filters.saturation)
        || !readFilterU8(&filters.exposure)
        || !readFilterU8(&filters.gamma)
        || !readFilterU8(&filters.blur)
        || !readFilterU8(&filters.posterizeRgb)
        || !readFilterU8(&filters.maskTolerance)
        || !readFilterU8(&filters.maskAmplify)
        || !readFilterU8(&filters.sobelEdges)
        || !readFilterU8(&filters.posterizeGray)
        || !readFilterU8(&filters.threshold)) {
        if (errorText)
            *errorText = QStringLiteral("Invalid filter block");
        return false;
    }
    int dither = 0;
    int contour = 0;
    int tone = 0;
    if (!readFilterU8(&dither)
        || !readFilterU8(&contour)
        || !readFilterU8(&tone)) {
        if (errorText)
            *errorText = QStringLiteral("Invalid filter modes");
        return false;
    }
    filters.ditherMode = static_cast<ImageFiltersPipeline::DitherMode>(dither);
    filters.contourMode = static_cast<ImageFiltersPipeline::ContourMode>(contour);
    filters.tonePreset = static_cast<ImageFiltersPipeline::TonePreset>(tone);

    quint8 r = 0;
    quint8 g = 0;
    quint8 b = 0;
    if (!readU8(payload, pos, &r) || !readU8(payload, pos, &g) || !readU8(payload, pos, &b))
        return false;
    filters.maskColor = QColor(r, g, b);
    session->filterParams = filters;
    return true;
}

bool readAssets(const QByteArray &payload, int &pos, StudioProject *project, QString *errorText)
{
    quint16 assetCount = 0;
    if (!readU16(payload, pos, &assetCount))
        return false;
    if (assetCount > 4096) {
        if (errorText)
            *errorText = QStringLiteral("Too many project assets");
        return false;
    }

    project->assets.clear();
    project->assets.reserve(assetCount);
    for (quint16 i = 0; i < assetCount; ++i) {
        QString path;
        QString name;
        qint16 ox = 0;
        qint16 oy = 0;
        if (!readString(payload, pos, &path)
            || !readString(payload, pos, &name)
            || !readS16(payload, pos, &ox)
            || !readS16(payload, pos, &oy)) {
            if (errorText)
                *errorText = QStringLiteral("Invalid asset entry");
            return false;
        }
        const QString decoded = StoredPath::decode(path);
        project->assets.append(ProjectAsset{
            decoded,
            name.isEmpty() ? QFileInfo(decoded).fileName() : name,
            ox,
            oy,
        });
    }
    return true;
}

bool readEmbeddedImages(const QByteArray &payload, int &pos, StudioProject *project, QString *errorText)
{
    project->resultPreviewPng.clear();
    project->sourceImagePng.clear();
    if (pos >= payload.size())
        return true;
    if (!readBlob(payload, pos, &project->resultPreviewPng)) {
        if (errorText)
            *errorText = QStringLiteral("Invalid preview blob");
        return false;
    }
    if (pos >= payload.size())
        return true;
    if (!readBlob(payload, pos, &project->sourceImagePng)) {
        if (errorText)
            *errorText = QStringLiteral("Invalid source image blob");
        return false;
    }
    if (pos < payload.size() && errorText)
        *errorText = QStringLiteral("Trailing project data");
    return true;
}

bool parsePayloadV1(const QByteArray &payload, StudioProject *project, QString *errorText)
{
    int pos = 0;
    QString legacyController;
    QString legacyExportTarget;
    QString legacyDriver;
    qint16 offsetX = 0;
    qint16 offsetY = 0;
    if (!readString(payload, pos, &project->name)
        || !readString(payload, pos, &legacyController)
        || !readString(payload, pos, &legacyExportTarget)
        || !readS16(payload, pos, &offsetX)
        || !readS16(payload, pos, &offsetY)) {
        if (errorText)
            *errorText = QStringLiteral("Invalid v1 project header");
        return false;
    }
    project->offsetX = offsetX;
    project->offsetY = offsetY;
    Q_UNUSED(legacyController);
    Q_UNUSED(legacyExportTarget);

    SessionSnapshot session;
    if (!readSessionBlock(payload, pos, 1, &session, errorText))
        return false;
    if (!readString(payload, pos, &legacyDriver))
        return false;
    Q_UNUSED(legacyDriver);

    project->session = session;
    if (!readAssets(payload, pos, project, errorText))
        return false;
    return readEmbeddedImages(payload, pos, project, errorText);
}

bool parsePayloadV2(const QByteArray &payload, StudioProject *project, QString *errorText)
{
    int pos = 0;
    qint16 offsetX = 0;
    qint16 offsetY = 0;
    if (!readString(payload, pos, &project->name)
        || !readS16(payload, pos, &offsetX)
        || !readS16(payload, pos, &offsetY)) {
        if (errorText)
            *errorText = QStringLiteral("Invalid v2 project header");
        return false;
    }
    project->offsetX = offsetX;
    project->offsetY = offsetY;

    SessionSnapshot session;
    if (!readSessionBlock(payload, pos, 2, &session, errorText))
        return false;

    project->session = session;
    if (!readAssets(payload, pos, project, errorText))
        return false;
    return readEmbeddedImages(payload, pos, project, errorText);
}

bool parsePayload(const QByteArray &payload, quint8 version, StudioProject *project, QString *errorText)
{
    if (version == 1)
        return parsePayloadV1(payload, project, errorText);
    if (version == 2)
        return parsePayloadV2(payload, project, errorText);
    if (errorText)
        *errorText = QStringLiteral("Unsupported payload version");
    return false;
}

} // namespace

bool ProjectFormatDecoder::decodeFile(const QByteArray &fileData, StudioProject *project, QString *errorText)
{
    if (!project) {
        if (errorText)
            *errorText = QStringLiteral("Invalid project");
        return false;
    }
    if (!ProjectFormat::isNativePayload(fileData)) {
        if (errorText)
            *errorText = QStringLiteral("Invalid project file");
        return false;
    }
    if (fileData.size() < 6) {
        if (errorText)
            *errorText = QStringLiteral("Truncated project file");
        return false;
    }

    const quint8 version = quint8(uchar(fileData.at(4)));
    if (version != 1 && version != 2) {
        if (errorText)
            *errorText = QStringLiteral("Unsupported project version");
        return false;
    }

    const quint8 flags = quint8(uchar(fileData.at(5)));
    QByteArray payload = fileData.mid(6);
    if ((flags & 0x01) != 0) {
        payload = qUncompress(payload);
        if (payload.isEmpty()) {
            if (errorText)
                *errorText = QStringLiteral("Failed to decompress project");
            return false;
        }
    }

    if (!parsePayload(payload, version, project, errorText)) {
        if (errorText && errorText->isEmpty())
            *errorText = QStringLiteral("Corrupt project data");
        qCWarning(lcPersistence) << "Project decode failed:" << (errorText ? *errorText : QString());
        return false;
    }
    if (project->name.isEmpty())
        project->name = QStringLiteral("Untitled");
    return true;
}
