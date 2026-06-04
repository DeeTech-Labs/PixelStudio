#include "persistence/ProjectFormat.h"

#include "persistence/ProjectService.h"
#include "persistence/StoredPath.h"

#include <QFileInfo>

namespace {

constexpr quint8 kFlagCompressed = 0x01;
constexpr int kCompressThreshold = 96;

void appendU8(QByteArray &out, quint8 v)
{
    out.append(char(v));
}

void appendU16(QByteArray &out, quint16 v)
{
    out.append(char(v & 0xff));
    out.append(char((v >> 8) & 0xff));
}

void appendS16(QByteArray &out, qint16 v)
{
    appendU16(out, quint16(v));
}

void appendString(QByteArray &out, const QString &s)
{
    const QByteArray utf8 = s.toUtf8();
    const quint16 len = quint16(qMin(utf8.size(), 0xffff));
    appendU16(out, len);
    out.append(utf8.constData(), len);
}

bool readU8(const QByteArray &in, int &pos, quint8 *v)
{
    if (pos >= in.size())
        return false;
    *v = quint8(uchar(in.at(pos++)));
    return true;
}

bool readU16(const QByteArray &in, int &pos, quint16 *v)
{
    if (pos + 2 > in.size())
        return false;
    const auto b = reinterpret_cast<const uchar *>(in.constData() + pos);
    *v = quint16(b[0]) | (quint16(b[1]) << 8);
    pos += 2;
    return true;
}

bool readS16(const QByteArray &in, int &pos, qint16 *v)
{
    quint16 u = 0;
    if (!readU16(in, pos, &u))
        return false;
    *v = qint16(u);
    return true;
}

bool readString(const QByteArray &in, int &pos, QString *s)
{
    quint16 len = 0;
    if (!readU16(in, pos, &len))
        return false;
    if (pos + len > in.size())
        return false;
    *s = QString::fromUtf8(in.constData() + pos, len);
    pos += len;
    return true;
}

quint32 sessionFlags(const SessionSnapshot &session)
{
    quint32 f = 0;
    if (session.dithering)
        f |= 1u << 0;
    if (session.flipHorizontal)
        f |= 1u << 1;
    if (session.flipVertical)
        f |= 1u << 2;
    if (session.invertMono)
        f |= 1u << 3;
    if (session.showGrid)
        f |= 1u << 4;
    if (session.codeIncludeComments)
        f |= 1u << 5;
    if (session.codeUseProgmem)
        f |= 1u << 6;
    if (session.codeStaticStorage)
        f |= 1u << 7;
    return f;
}

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
}

quint32 filterFlags(const ImageFiltersPipeline::Params &p)
{
    quint32 f = 0;
    if (p.blackBackground)
        f |= 1u << 0;
    if (p.colorMaskEnabled)
        f |= 1u << 1;
    if (p.sharpen)
        f |= 1u << 2;
    if (p.thresholdEnabled)
        f |= 1u << 3;
    if (p.invert)
        f |= 1u << 4;
    return f;
}

void applyFilterFlags(quint32 f, ImageFiltersPipeline::Params *p)
{
    p->blackBackground = (f >> 0) & 1;
    p->colorMaskEnabled = (f >> 1) & 1;
    p->sharpen = (f >> 2) & 1;
    p->thresholdEnabled = (f >> 3) & 1;
    p->invert = (f >> 4) & 1;
}

quint8 clampU8(int v)
{
    return quint8(qBound(0, v, 255));
}

QByteArray buildPayload(const StudioProject &project)
{
    QByteArray out;
    out.reserve(256);

    appendString(out, project.name);
    appendS16(out, qint16(project.offsetX));
    appendS16(out, qint16(project.offsetY));

    const SessionSnapshot &s = project.session;
    appendU16(out, quint16(sessionFlags(s) & 0xffff));
    appendU8(out, clampU8(s.scaleMode));
    appendU8(out, clampU8(s.monoLayout));
    appendU8(out, clampU8(s.rotation));
    appendU8(out, clampU8(s.encodingMode));
    appendU8(out, clampU8(s.gridThresholdZoom));
    appendU16(out, quint16(qBound(0, s.displayWidth, 0xffff)));
    appendU16(out, quint16(qBound(0, s.displayHeight, 0xffff)));
    appendU16(out, quint16(qBound(0, s.monoThreshold, 0xffff)));
    appendString(out, s.profileId);
    appendString(out, s.arrayName);

    const ImageFiltersPipeline::Params &f = s.filterParams;
    appendU16(out, quint16(filterFlags(f) & 0xffff));
    appendU8(out, clampU8(f.brightness));
    appendU8(out, clampU8(f.contrast));
    appendU8(out, clampU8(f.saturation));
    appendU8(out, clampU8(f.exposure));
    appendU8(out, clampU8(f.gamma));
    appendU8(out, clampU8(f.blur));
    appendU8(out, clampU8(f.posterizeRgb));
    appendU8(out, clampU8(f.maskTolerance));
    appendU8(out, clampU8(f.maskAmplify));
    appendU8(out, clampU8(f.sobelEdges));
    appendU8(out, clampU8(f.posterizeGray));
    appendU8(out, clampU8(f.threshold));
    appendU8(out, clampU8(static_cast<int>(f.ditherMode)));
    appendU8(out, clampU8(static_cast<int>(f.contourMode)));
    appendU8(out, clampU8(static_cast<int>(f.tonePreset)));
    const QRgb rgb = f.maskColor.rgb();
    appendU8(out, quint8(qRed(rgb)));
    appendU8(out, quint8(qGreen(rgb)));
    appendU8(out, quint8(qBlue(rgb)));

    appendU16(out, quint16(qMin(project.assets.size(), 0xffff)));
    for (const ProjectAsset &asset : project.assets) {
        appendString(out, StoredPath::encode(asset.path));
        appendString(out, asset.name);
        appendS16(out, qint16(asset.offsetX));
        appendS16(out, qint16(asset.offsetY));
    }
    return out;
}

bool readSessionBlock(const QByteArray &payload, int &pos, SessionSnapshot *session)
{
    quint16 sessionBits = 0;
    quint8 u8 = 0;
    quint16 u16 = 0;
    if (!readU16(payload, pos, &sessionBits)
        || !readU8(payload, pos, &u8)) {
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
    session->encodingMode = u8;
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
        return false;
    }
    int dither = 0;
    int contour = 0;
    int tone = 0;
    if (!readFilterU8(&dither)
        || !readFilterU8(&contour)
        || !readFilterU8(&tone)) {
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

bool readAssets(const QByteArray &payload, int &pos, StudioProject *project)
{
    quint16 assetCount = 0;
    if (!readU16(payload, pos, &assetCount))
        return false;

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

bool parsePayloadV1(const QByteArray &payload, StudioProject *project)
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
        return false;
    }
    project->offsetX = offsetX;
    project->offsetY = offsetY;
    Q_UNUSED(legacyController);
    Q_UNUSED(legacyExportTarget);

    SessionSnapshot session;
    if (!readSessionBlock(payload, pos, &session))
        return false;
    if (!readString(payload, pos, &legacyDriver))
        return false;
    Q_UNUSED(legacyDriver);

    project->session = session;
    return readAssets(payload, pos, project);
}

bool parsePayloadV2(const QByteArray &payload, StudioProject *project)
{
    int pos = 0;
    qint16 offsetX = 0;
    qint16 offsetY = 0;
    if (!readString(payload, pos, &project->name)
        || !readS16(payload, pos, &offsetX)
        || !readS16(payload, pos, &offsetY)) {
        return false;
    }
    project->offsetX = offsetX;
    project->offsetY = offsetY;

    SessionSnapshot session;
    if (!readSessionBlock(payload, pos, &session))
        return false;

    project->session = session;
    return readAssets(payload, pos, project);
}

bool parsePayload(const QByteArray &payload, quint8 version, StudioProject *project)
{
    if (version == 1)
        return parsePayloadV1(payload, project);
    if (version == 2)
        return parsePayloadV2(payload, project);
    return false;
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
    const QByteArray payload = buildPayload(project);
    QByteArray file;
    file.reserve(8 + payload.size());
    file.append(kMagic, 4);
    appendU8(file, kVersion);

    quint8 flags = 0;
    QByteArray body = payload;
    if (payload.size() >= kCompressThreshold) {
        body = qCompress(payload, 9);
        flags |= kFlagCompressed;
    }
    appendU8(file, flags);
    file.append(body);
    return file;
}

bool ProjectFormat::decode(const QByteArray &fileData, StudioProject *project, QString *errorText)
{
    if (!project) {
        if (errorText)
            *errorText = QStringLiteral("Invalid project");
        return false;
    }
    if (!isNativePayload(fileData)) {
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
    if ((flags & kFlagCompressed) != 0) {
        payload = qUncompress(payload);
        if (payload.isEmpty()) {
            if (errorText)
                *errorText = QStringLiteral("Failed to decompress project");
            return false;
        }
    }

    if (!parsePayload(payload, version, project)) {
        if (errorText)
            *errorText = QStringLiteral("Corrupt project data");
        return false;
    }
    if (project->name.isEmpty())
        project->name = QStringLiteral("Untitled");
    return true;
}
