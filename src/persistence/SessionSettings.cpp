#include "persistence/SessionSettings.h"

#include "persistence/AppPaths.h"
#include "persistence/SettingsSchema.h"
#include "persistence/StoredPath.h"

#include <QFileInfo>
#include <QUrl>

namespace {

constexpr int kMaxRecent = 10;

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

void readFilterParams(const QSettings &s, ImageFiltersPipeline::Params *p)
{
    p->blackBackground = s.value(QStringLiteral("filter/blackBg"), false).toBool();
    p->brightness = s.value(QStringLiteral("filter/brightness"), 100).toInt();
    p->contrast = s.value(QStringLiteral("filter/contrast"), 100).toInt();
    p->saturation = s.value(QStringLiteral("filter/saturation"), 100).toInt();
    p->exposure = s.value(QStringLiteral("filter/exposure"), 100).toInt();
    p->gamma = s.value(QStringLiteral("filter/gamma"), 100).toInt();
    p->blur = s.value(QStringLiteral("filter/blur"), 0).toInt();
    p->posterizeRgb = s.value(QStringLiteral("filter/posterizeRgb"), 0).toInt();
    p->colorMaskEnabled = s.value(QStringLiteral("filter/maskEnabled"), false).toBool();
    p->maskColor = s.value(QStringLiteral("filter/maskColor"), QColor(Qt::black)).value<QColor>();
    p->maskTolerance = s.value(QStringLiteral("filter/maskTolerance"), 0).toInt();
    p->maskAmplify = s.value(QStringLiteral("filter/maskAmplify"), 1).toInt();
    p->sharpen = s.value(QStringLiteral("filter/sharpen"), false).toBool();
    p->sobelEdges = s.value(QStringLiteral("filter/sobel"), 0).toInt();
    p->posterizeGray = s.value(QStringLiteral("filter/posterizeGray"), 0).toInt();
    p->ditherMode = static_cast<ImageFiltersPipeline::DitherMode>(
        s.value(QStringLiteral("filter/ditherMode"), 1).toInt());
    p->contourMode = static_cast<ImageFiltersPipeline::ContourMode>(
        s.value(QStringLiteral("filter/contourMode"), 0).toInt());
    p->tonePreset = static_cast<ImageFiltersPipeline::TonePreset>(
        s.value(QStringLiteral("filter/tonePreset"), 0).toInt());
}

void writeFilterParams(QSettings &s, const ImageFiltersPipeline::Params &p)
{
    s.setValue(QStringLiteral("filter/blackBg"), p.blackBackground);
    s.setValue(QStringLiteral("filter/brightness"), p.brightness);
    s.setValue(QStringLiteral("filter/contrast"), p.contrast);
    s.setValue(QStringLiteral("filter/saturation"), p.saturation);
    s.setValue(QStringLiteral("filter/exposure"), p.exposure);
    s.setValue(QStringLiteral("filter/gamma"), p.gamma);
    s.setValue(QStringLiteral("filter/blur"), p.blur);
    s.setValue(QStringLiteral("filter/posterizeRgb"), p.posterizeRgb);
    s.setValue(QStringLiteral("filter/maskEnabled"), p.colorMaskEnabled);
    s.setValue(QStringLiteral("filter/maskColor"), p.maskColor);
    s.setValue(QStringLiteral("filter/maskTolerance"), p.maskTolerance);
    s.setValue(QStringLiteral("filter/maskAmplify"), p.maskAmplify);
    s.setValue(QStringLiteral("filter/sharpen"), p.sharpen);
    s.setValue(QStringLiteral("filter/sobel"), p.sobelEdges);
    s.setValue(QStringLiteral("filter/posterizeGray"), p.posterizeGray);
    s.setValue(QStringLiteral("filter/ditherMode"), static_cast<int>(p.ditherMode));
    s.setValue(QStringLiteral("filter/contourMode"), static_cast<int>(p.contourMode));
    s.setValue(QStringLiteral("filter/tonePreset"), static_cast<int>(p.tonePreset));
}

} // namespace

SessionSettings::SessionSettings(QObject *parent)
    : QObject(parent)
    , m_settings(AppPaths::sessionSettingsFile(), QSettings::IniFormat)
{
    SettingsSchema::migrateSessionSettings(m_settings);
    m_settings.sync();
}

void SessionSettings::syncNow()
{
    m_settings.sync();
}

void SessionSettings::load(SessionSnapshot *snapshot) const
{
    if (!snapshot)
        return;

    snapshot->profileId = m_settings.value(QStringLiteral("profileId"), snapshot->profileId).toString();
    snapshot->displayWidth = m_settings.value(QStringLiteral("displayWidth"), snapshot->displayWidth).toInt();
    snapshot->displayHeight = m_settings.value(QStringLiteral("displayHeight"), snapshot->displayHeight).toInt();
    snapshot->scaleMode = m_settings.value(QStringLiteral("scaleMode"), snapshot->scaleMode).toInt();
    snapshot->dithering = m_settings.value(QStringLiteral("dithering"), snapshot->dithering).toBool();
    snapshot->monoThreshold = m_settings.value(QStringLiteral("monoThreshold"), snapshot->monoThreshold).toInt();
    snapshot->arrayName = m_settings.value(QStringLiteral("arrayName"), snapshot->arrayName).toString();
    snapshot->encodingMode = m_settings.value(QStringLiteral("encodingMode"), snapshot->encodingMode).toInt();
    snapshot->monoLayout = m_settings.value(QStringLiteral("monoLayout"), snapshot->monoLayout).toInt();
    snapshot->rotation = m_settings.value(QStringLiteral("rotation"), 0).toInt();
    snapshot->flipHorizontal = m_settings.value(QStringLiteral("flipH"), false).toBool();
    snapshot->flipVertical = m_settings.value(QStringLiteral("flipV"), false).toBool();
    snapshot->invertMono = m_settings.value(QStringLiteral("invertMono"), false).toBool();
    snapshot->showGrid = m_settings.value(QStringLiteral("showGrid"), false).toBool();
    snapshot->gridThresholdZoom = m_settings.value(QStringLiteral("gridZoom"), 8).toInt();
    snapshot->codeIncludeComments = m_settings.value(QStringLiteral("code/includeComments"), true).toBool();
    snapshot->codeUseProgmem = m_settings.value(QStringLiteral("code/useProgmem"), true).toBool();
    snapshot->codeStaticStorage = m_settings.value(QStringLiteral("code/staticStorage"), true).toBool();
    readFilterParams(m_settings, &snapshot->filterParams);
}

void SessionSettings::save(const SessionSnapshot &snapshot)
{
    m_settings.setValue(QStringLiteral("profileId"), snapshot.profileId);
    m_settings.setValue(QStringLiteral("displayWidth"), snapshot.displayWidth);
    m_settings.setValue(QStringLiteral("displayHeight"), snapshot.displayHeight);
    m_settings.setValue(QStringLiteral("scaleMode"), snapshot.scaleMode);
    m_settings.setValue(QStringLiteral("dithering"), snapshot.dithering);
    m_settings.setValue(QStringLiteral("monoThreshold"), snapshot.monoThreshold);
    m_settings.setValue(QStringLiteral("arrayName"), snapshot.arrayName);
    m_settings.setValue(QStringLiteral("encodingMode"), snapshot.encodingMode);
    m_settings.setValue(QStringLiteral("monoLayout"), snapshot.monoLayout);
    m_settings.setValue(QStringLiteral("rotation"), snapshot.rotation);
    m_settings.setValue(QStringLiteral("flipH"), snapshot.flipHorizontal);
    m_settings.setValue(QStringLiteral("flipV"), snapshot.flipVertical);
    m_settings.setValue(QStringLiteral("invertMono"), snapshot.invertMono);
    m_settings.setValue(QStringLiteral("showGrid"), snapshot.showGrid);
    m_settings.setValue(QStringLiteral("gridZoom"), snapshot.gridThresholdZoom);
    m_settings.setValue(QStringLiteral("code/includeComments"), snapshot.codeIncludeComments);
    m_settings.setValue(QStringLiteral("code/useProgmem"), snapshot.codeUseProgmem);
    m_settings.setValue(QStringLiteral("code/staticStorage"), snapshot.codeStaticStorage);
    writeFilterParams(m_settings, snapshot.filterParams);
    syncNow();
}

void SessionSettings::loadUiState(SessionUiState *state) const
{
    if (!state)
        return;
    state->lastProjectFile = StoredPath::decode(m_settings.value(QStringLiteral("ui/lastProjectFile")).toString());
    state->lastOpenImageDir = StoredPath::decode(m_settings.value(QStringLiteral("ui/lastOpenImageDir")).toString());
    state->lastExportDir = StoredPath::decode(m_settings.value(QStringLiteral("ui/lastExportDir")).toString());
    state->watchInputFolder = StoredPath::decode(m_settings.value(QStringLiteral("ui/watchInput")).toString());
    state->watchOutputFolder = StoredPath::decode(m_settings.value(QStringLiteral("ui/watchOutput")).toString());
    state->watchActive = m_settings.value(QStringLiteral("ui/watchActive"), false).toBool();
}

void SessionSettings::saveUiState(const SessionUiState &state)
{
    m_settings.setValue(QStringLiteral("ui/lastProjectFile"), StoredPath::encode(state.lastProjectFile));
    m_settings.setValue(QStringLiteral("ui/lastOpenImageDir"), StoredPath::encode(state.lastOpenImageDir));
    m_settings.setValue(QStringLiteral("ui/lastExportDir"), StoredPath::encode(state.lastExportDir));
    m_settings.setValue(QStringLiteral("ui/watchInput"), StoredPath::encode(state.watchInputFolder));
    m_settings.setValue(QStringLiteral("ui/watchOutput"), StoredPath::encode(state.watchOutputFolder));
    m_settings.setValue(QStringLiteral("ui/watchActive"), state.watchActive);
    syncNow();
}

void SessionSettings::pruneMissingRecentFiles()
{
    const QStringList stored = m_settings.value(QStringLiteral("recentFiles")).toStringList();
    QStringList kept;
    kept.reserve(stored.size());
    for (const QString &entry : stored) {
        const QString path = StoredPath::decode(entry);
        if (QFileInfo::exists(path))
            kept.append(entry);
    }
    if (kept == stored)
        return;
    m_settings.setValue(QStringLiteral("recentFiles"), kept);
    syncNow();
    emit recentFilesChanged();
}

void SessionSettings::addRecentFile(const QUrl &url)
{
    QString path = url.toLocalFile();
    if (path.isEmpty())
        path = url.toString(QUrl::PreferLocalFile);
    if (path.isEmpty())
        return;

    const QString canonical = QFileInfo(path).canonicalFilePath();
    const QString keyPath = canonical.isEmpty() ? path : canonical;
    const QString encoded = StoredPath::encode(keyPath);
    QStringList recent = m_settings.value(QStringLiteral("recentFiles")).toStringList();
    for (auto it = recent.begin(); it != recent.end();) {
        const QString stored = StoredPath::decode(*it);
        const QString storedCanonical = QFileInfo(stored).canonicalFilePath();
        const QString storedKey = storedCanonical.isEmpty() ? stored : storedCanonical;
        if (storedKey == keyPath || stored == path || stored == keyPath)
            it = recent.erase(it);
        else
            ++it;
    }
    recent.prepend(encoded);
    while (recent.size() > kMaxRecent)
        recent.removeLast();
    m_settings.setValue(QStringLiteral("recentFiles"), recent);
    syncNow();
    emit recentFilesChanged();
}

QVariantList SessionSettings::recentFiles() const
{
    const QStringList stored = m_settings.value(QStringLiteral("recentFiles")).toStringList();
    QVariantList out;
    out.reserve(stored.size());
    QStringList seenKeys;
    for (const QString &entry : stored) {
        const QString path = StoredPath::decode(entry);
        if (!QFileInfo::exists(path))
            continue;
        const QString canonical = QFileInfo(path).canonicalFilePath();
        const QString key = canonical.isEmpty() ? path : canonical;
        if (seenKeys.contains(key))
            continue;
        seenKeys.append(key);

        QVariantMap row;
        row.insert(QStringLiteral("path"), path);
        row.insert(QStringLiteral("name"), QFileInfo(path).fileName());
        if (isRasterImagePath(path))
            row.insert(QStringLiteral("thumbnailUrl"), QUrl::fromLocalFile(path).toString());
        out.append(row);
    }
    return out;
}

void SessionSettings::addRecentExport(const QString &absolutePath)
{
    if (absolutePath.isEmpty())
        return;
    const QString encoded = StoredPath::encode(absolutePath);
    QStringList recent = m_settings.value(QStringLiteral("recentExports")).toStringList();
    recent.removeAll(encoded);
    recent.removeAll(absolutePath);
    recent.prepend(encoded);
    while (recent.size() > kMaxRecent)
        recent.removeLast();
    m_settings.setValue(QStringLiteral("recentExports"), recent);
    syncNow();
    emit recentExportsChanged();
}

void SessionSettings::pruneMissingRecentExports()
{
    const QStringList stored = m_settings.value(QStringLiteral("recentExports")).toStringList();
    QStringList kept;
    for (const QString &entry : stored) {
        if (QFileInfo::exists(StoredPath::decode(entry)))
            kept.append(entry);
    }
    if (kept == stored)
        return;
    m_settings.setValue(QStringLiteral("recentExports"), kept);
    syncNow();
    emit recentExportsChanged();
}

QVariantList SessionSettings::recentExports() const
{
    const QStringList stored = m_settings.value(QStringLiteral("recentExports")).toStringList();
    QVariantList out;
    for (const QString &entry : stored) {
        const QString path = StoredPath::decode(entry);
        if (!QFileInfo::exists(path))
            continue;
        QVariantMap row;
        row.insert(QStringLiteral("path"), path);
        row.insert(QStringLiteral("name"), QFileInfo(path).fileName());
        out.append(row);
    }
    return out;
}

void SessionSettings::resetToDefaults()
{
    m_settings.clear();
    SettingsSchema::migrateSessionSettings(m_settings);
    syncNow();
    emit recentFilesChanged();
    emit recentExportsChanged();
}
