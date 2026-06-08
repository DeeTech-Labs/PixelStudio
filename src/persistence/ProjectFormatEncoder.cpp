#include "persistence/ProjectFormatEncoder.h"

#include "persistence/ProjectFormat.h"
#include "persistence/ProjectFormatIo.h"
#include "persistence/ProjectService.h"
#include "persistence/StoredPath.h"
#include "persistence/SessionSettings.h"
#include "processing/ImageFiltersPipeline.h"

#include <QColor>

namespace {

using namespace ProjectFormatIo;

constexpr quint8 kFlagCompressed = 0x01;
constexpr int kCompressThreshold = 96;

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
    if (session.rgb565BigEndian)
        f |= 1u << 8;
    if (session.linearColorSpace)
        f |= 1u << 9;
    const int dma = session.codeDmaAlign == 8 ? 2 : (session.codeDmaAlign == 4 ? 1 : 0);
    f |= quint32(dma & 0x3) << 10;
    return f;
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

quint8 clampU8(int v)
{
    return quint8(qBound(0, v, 255));
}

} // namespace

QByteArray ProjectFormatEncoder::buildPayload(const StudioProject &project)
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
    appendBlob(out, project.resultPreviewPng);
    appendBlob(out, project.sourceImagePng);
    return out;
}

QByteArray ProjectFormatEncoder::encodeFile(const StudioProject &project)
{
    const QByteArray payload = buildPayload(project);
    QByteArray file;
    file.reserve(8 + payload.size());
    file.append(ProjectFormat::kMagic, 4);
    appendU8(file, ProjectFormat::kVersion);

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
