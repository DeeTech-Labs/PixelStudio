#include "app/studio/DisplayConverter.h"
#include "app/preview/PreviewImageProvider.h"
#include "app/studio/model/ConverterEncoding.h"
#include "app/studio/pipeline/ImagePipelineController.h"
#include "app/studio/TabStateService.h"
#include "app/tabs/TabPreviewSlots.h"
#include "translation/AppLocale.h"
#include "persistence/AppSettings.h"
#include "processing/DisplayCodeGenerator.h"
#include "processing/PixelFormatCatalog.h"
#include "io/ImageLoader.h"
#include "persistence/SessionSettings.h"
#include "processing/ConvertPipeline.h"
#include "processing/PreviewPalette.h"

#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QVariantMap>

namespace {

QString previewSlotForTab(const QString &tabId, const char *role)
{
    if (tabId.isEmpty())
        return QString::fromLatin1(role);
    if (qstrcmp(role, "source") == 0)
        return TabPreviewSlots::source(tabId);
    if (qstrcmp(role, "preview") == 0)
        return TabPreviewSlots::preview(tabId);
    return TabPreviewSlots::process(tabId);
}

QUrl publishPreview(PreviewImageProvider *provider, const QString &tabId, const char *role, const QImage &img)
{
    if (img.isNull() || !provider)
        return {};
    const QString slot = previewSlotForTab(tabId, role);
    provider->setImage(slot, img);
    return provider->imageUrl(slot);
}

} // namespace

DisplayConverter::DisplayConverter(SessionSettings *session, AppSettings *appSettings, QObject *parent)
    : QObject(parent)
    , m_loader(new ImageLoader(this))
    , m_session(session)
    , m_appSettings(appSettings)
    , m_filters(this)
    , m_transform(this)
    , m_displayOutput(this)
    , m_codeGen(this)
    , m_viewport(this)
    , m_project(this)
    , m_export(this)
{
    m_filters.attach(this, &m_state);
    m_transform.attach(this, &m_state);
    m_displayOutput.attach(this, &m_state);
    m_codeGen.attach(this, &m_state);
    m_viewport.attach(this, &m_state);
    m_project.attach(this, &m_state, m_session);
    m_export.attach(this, &m_state, m_session);
    connect(&m_project, &ProjectController::errorOccurred, this, &DisplayConverter::errorOccurred);
    connect(&m_export, &ExportController::errorOccurred, this, &DisplayConverter::errorOccurred);
    connect(&m_filters,
            &ImageFilterController::hostInvertMonoChanged,
            &m_transform,
            &ImageTransformController::notifyInvertMonoChanged);
    connect(m_loader, &ImageLoader::loaded, this, &DisplayConverter::onImageLoaded);
    connect(m_loader, &ImageLoader::error, this, &DisplayConverter::errorOccurred);
    connect(m_loader, &ImageLoader::loadingChanged, this, [this]() {
        const bool loading = m_loader->loading();
        if (m_state.imageLoading == loading)
            return;
        m_state.imageLoading = loading;
        emit imageLoadingChanged();
    });
    m_state.filterParams.ditherMode = ImageFiltersPipeline::DitherMode::FloydSteinberg;
    m_state.filterParams.threshold = m_state.monoThreshold;
    m_rebuildDebounceTimer.setSingleShot(true);
    m_rebuildDebounceTimer.setInterval(200);
    connect(&m_rebuildDebounceTimer, &QTimer::timeout, this, [this]() {
        ImagePipelineController::startAsyncRebuild(*this);
    });
    connect(&m_rebuildWatcher, &QFutureWatcher<ConverterAsyncBuildResult>::finished,
            this, [this]() { ImagePipelineController::onAsyncRebuildFinished(*this); });
    m_sessionSaveTimer.setSingleShot(true);
    m_sessionSaveTimer.setInterval(400);
    connect(&m_sessionSaveTimer, &QTimer::timeout, this, &DisplayConverter::persistSession);
    m_autosaveTimer.setSingleShot(false);
    connect(&m_autosaveTimer, &QTimer::timeout, this, &DisplayConverter::onAutosaveTimeout);
    if (m_appSettings) {
        connect(m_appSettings, &AppSettings::projectAutosaveChanged, this, &DisplayConverter::restartAutosaveTimer);
        connect(m_appSettings, &AppSettings::projectAutosaveSecondsChanged, this, &DisplayConverter::restartAutosaveTimer);
    }
    if (m_session) {
        loadPersistedSession();
        m_session->loadUiState(&m_state.uiState);
        m_session->pruneMissingRecentFiles();
        m_session->pruneMissingRecentExports();
        m_export.applyStoredWatchState();
        updateWatchExportPrefix();
        restartAutosaveTimer();
    }
}

void DisplayConverter::setPreviewProvider(PreviewImageProvider *provider)
{
    m_previewProvider = provider;
}

void DisplayConverter::setActiveTabId(const QString &tabId)
{
    m_activeTabId = tabId;
}

void DisplayConverter::notifySourceGeometryChanged()
{
    if (!hasImage())
        return;
    refreshSourcePreview();
    emit sourceWidthChanged();
    emit sourceHeightChanged();
}

void DisplayConverter::notifyAllToolsChanged()
{
    m_transform.notifyAllChanged();
    m_displayOutput.notifyAllChanged();
    m_codeGen.notifyAllChanged();
    m_viewport.notifyAllChanged();
    m_filters.syncFromState();
}

int DisplayConverter::sourceWidth() const
{
    const QImage img = orientedSource();
    return img.isNull() ? 0 : img.width();
}

int DisplayConverter::sourceHeight() const
{
    const QImage img = orientedSource();
    return img.isNull() ? 0 : img.height();
}

QImage DisplayConverter::orientedSource() const
{
    if (m_state.sourceImage.isNull())
        return {};
    if (!m_state.orientedDirty && !m_state.orientedCache.isNull())
        return m_state.orientedCache;

    m_state.orientedCache = ConvertPipeline::applyOrientation(m_state.sourceImage, pipelineParams());
    m_state.orientedDirty = false;
    return m_state.orientedCache;
}

QVariantList DisplayConverter::previewPalette() const
{
    if (!m_state.lastResult.indexedPalette.isEmpty())
        return PreviewPalette::fromRgbList(m_state.lastResult.indexedPalette);
    if (!m_state.lastResult.preview.isNull())
        return PreviewPalette::fromPreviewImage(m_state.lastResult.preview);
    return {};
}

int DisplayConverter::previewColorCount() const
{
    return previewPalette().size();
}

QString DisplayConverter::imageFormatName() const
{
    if (!hasImage())
        return QString();
    const QString path = m_state.sourceFilePath;
    const int dot = path.lastIndexOf(QLatin1Char('.'));
    if (dot >= 0 && dot < path.size() - 1)
        return path.mid(dot + 1).toUpper();
    return AppLocale::tr("Image");
}

bool DisplayConverter::loadImage(const QUrl &url)
{
    if (url.scheme().startsWith(QStringLiteral("http"), Qt::CaseInsensitive)) {
        loadFromUrl(url.toString());
        return true;
    }
    m_loader->loadFromFileAsync(url);
    return true;
}

bool DisplayConverter::loadFromClipboard()
{
    QImage img;
    if (!m_loader->loadFromClipboard(img))
        return false;
    onImageLoaded(img, QUrl(QStringLiteral("clipboard://image")));
    return true;
}

void DisplayConverter::loadFromUrl(const QString &urlString)
{
    m_loader->loadFromUrl(urlString);
}

void DisplayConverter::detachActiveTab()
{
    m_rebuildDebounceTimer.stop();
    m_state.rebuildPending = false;
    m_state.lastAppliedGeneration = ++m_state.nextGeneration;
    m_activeTabId.clear();
}

void DisplayConverter::clear()
{
    m_rebuildDebounceTimer.stop();
    m_state.rebuildPending = false;
    m_state.lastAppliedGeneration = ++m_state.nextGeneration;
    m_activeTabId.clear();
    m_export.resetBatchExport();
    m_state.sourceImage = QImage();
    m_state.sourceFilePath.clear();
    m_state.sourcePath.clear();
    m_state.previewPath.clear();
    m_state.processPreviewPath.clear();
    m_state.generatedCode.clear();
    m_state.flashReport.clear();
    m_state.lastResult = {};
    m_state.rotation = 0;
    m_state.flipHorizontal = false;
    m_state.flipVertical = false;
    m_state.invertMono = false;
    m_state.filterParams = ImageFiltersPipeline::Params{};
    m_state.filterParams.threshold = m_state.monoThreshold;
    m_state.filterParams.ditherMode = m_state.dithering
        ? ImageFiltersPipeline::DitherMode::FloydSteinberg
        : ImageFiltersPipeline::DitherMode::None;
    markOrientedDirty();
    emit sourcePathChanged();
    emit previewPathChanged();
    emit processPreviewPathChanged();
    emit hasImageChanged();
    emit sourceWidthChanged();
    emit sourceHeightChanged();
    emit generatedCodeChanged();
    emit flashReportChanged();
    notifyAllToolsChanged();
    m_displayOutput.notifyFootprintChanged();
    ImagePipelineController::updateCodePreview(m_state, *this);
}

void DisplayConverter::onImageLoaded(const QImage &image, const QUrl &sourceUrl)
{
    m_state.sourceImage = image;
    m_state.rotation = 0;
    m_state.flipHorizontal = false;
    m_state.flipVertical = false;
    markOrientedDirty();
    if (sourceUrl.isLocalFile()) {
        const QString local = sourceUrl.toLocalFile();
        m_state.sourceFilePath = local.isEmpty() ? sourceUrl.path() : local;
    } else {
        m_state.sourceFilePath.clear();
    }
    refreshSourcePreview();
    emit hasImageChanged();
    m_transform.notifyAllChanged();
    emit sourceWidthChanged();
    emit sourceHeightChanged();
    if (m_session && sourceUrl.isLocalFile())
        m_session->addRecentFile(sourceUrl);
    if (sourceUrl.isLocalFile())
        m_project.rememberOpenImageDir(sourceUrl.toLocalFile().isEmpty() ? sourceUrl.path() : sourceUrl.toLocalFile());
    scheduleRebuild(true);
}

void DisplayConverter::rebuild()
{
    scheduleRebuild(true);
}

void DisplayConverter::scheduleRebuild(bool immediate)
{
    ImagePipelineController::scheduleRebuild(*this, immediate);
}

void DisplayConverter::applyPipelineResult(const ConverterAsyncBuildResult &result)
{
    if (result.generation < m_state.lastAppliedGeneration)
        return;

    m_state.lastAppliedGeneration = result.generation;
    m_state.lastResult = result.result;
    if (m_state.lastResult.preview.isNull()) {
        m_state.previewPath.clear();
        m_state.processPreviewPath.clear();
        m_state.generatedCode.clear();
        emit previewPathChanged();
        emit processPreviewPathChanged();
        emit generatedCodeChanged();
        ImagePipelineController::updateCodePreview(m_state, *this);
        ImagePipelineController::updateFlashReport(m_state, *this);
        m_displayOutput.notifyFootprintChanged();
        return;
    }

    m_state.processPreviewPath = publishPreview(m_previewProvider,
                                                m_activeTabId,
                                                "process",
                                                m_state.lastResult.processPreview);
    emit processPreviewPathChanged();
    m_state.previewPath = publishPreview(m_previewProvider,
                                         m_activeTabId,
                                         "preview",
                                         m_state.lastResult.preview);
    emit previewPathChanged();
    m_state.generatedCode = result.generatedCode;
    emit generatedCodeChanged();
    ImagePipelineController::updateCodePreview(m_state, *this);
    ImagePipelineController::updateFlashReport(m_state, *this);
    m_displayOutput.notifyFootprintChanged();
}

StudioProject DisplayConverter::projectSnapshot() const
{
    return captureTabState().project;
}

void DisplayConverter::applyProject(const StudioProject &project)
{
    m_project.applyProject(project);
}

void DisplayConverter::markOrientedDirty()
{
    m_state.orientedDirty = true;
    m_state.orientedCache = QImage();
}

void DisplayConverter::refreshSourcePreview()
{
    if (m_state.sourceImage.isNull())
        return;
    m_state.sourcePath = publishPreview(m_previewProvider, m_activeTabId, "source", orientedSource());
    emit sourcePathChanged();
}

ConverterTabSnapshot DisplayConverter::captureTabState(const QString &tabId) const
{
    const QString id = tabId.isEmpty() ? m_activeTabId : tabId;
    return TabStateService::capture(*this, id);
}

void DisplayConverter::restoreTabState(const ConverterTabSnapshot &snapshot)
{
    TabStateService::restore(*this, snapshot);
}

StudioProject DisplayConverter::projectSnapshotForDisk() const
{
    return m_project.projectSnapshotForDisk();
}

ConvertPipelineParams DisplayConverter::pipelineParams() const
{
    ConvertPipelineParams p;
    p.displayWidth = m_state.displayWidth;
    p.displayHeight = m_state.displayHeight;
    p.colorMode = m_state.colorMode;
    p.scaleMode = m_state.scaleMode;
    p.monoThreshold = m_state.monoThreshold;
    p.invertMono = m_state.invertMono;
    p.rotation = m_state.rotation;
    p.flipHorizontal = m_state.flipHorizontal;
    p.flipVertical = m_state.flipVertical;
    p.filterParams = m_state.filterParams;
    p.encodingMode = m_state.encodingMode;
    p.linearColorSpace = m_state.linearColorSpace;
    return p;
}

SessionSnapshot DisplayConverter::sessionSnapshot() const
{
    SessionSnapshot s;
    s.profileId = m_state.profileId;
    s.displayWidth = m_state.displayWidth;
    s.displayHeight = m_state.displayHeight;
    s.scaleMode = static_cast<int>(m_state.scaleMode);
    s.dithering = m_state.dithering;
    s.monoThreshold = m_state.monoThreshold;
    s.arrayName = m_state.arrayName;
    s.encodingMode = static_cast<int>(m_state.encodingMode);
    s.monoLayout = static_cast<int>(m_state.monoLayout);
    s.rotation = m_state.rotation;
    s.flipHorizontal = m_state.flipHorizontal;
    s.flipVertical = m_state.flipVertical;
    s.invertMono = m_state.invertMono;
    s.showGrid = m_state.showGrid;
    s.gridThresholdZoom = m_state.gridThresholdZoom;
    s.filterParams = m_state.filterParams;
    s.codeIncludeComments = m_state.codeGenOptions.includeHeaderComments;
    s.codeUseProgmem = m_state.codeGenOptions.useProgmem;
    s.codeStaticStorage = m_state.codeGenOptions.staticStorage;
    s.rgb565BigEndian = m_state.codeGenOptions.rgb565BigEndian;
    s.codeDmaAlign = m_state.codeGenOptions.dmaPaddingAlign;
    s.linearColorSpace = m_state.linearColorSpace;
    return s;
}

void DisplayConverter::applySessionSnapshot(const SessionSnapshot &snapshot)
{
    m_state.profileId = snapshot.profileId;
    m_state.displayWidth = snapshot.displayWidth;
    m_state.displayHeight = snapshot.displayHeight;
    m_state.scaleMode = static_cast<DisplayProfile::ScaleMode>(qBound(0, snapshot.scaleMode, 2));
    m_state.dithering = snapshot.dithering;
    m_state.monoThreshold = snapshot.monoThreshold;
    m_state.arrayName = snapshot.arrayName;
    const int storedEncoding = snapshot.encodingMode;
    if (storedEncoding >= 0
        && storedEncoding < static_cast<int>(DisplayCodeGenerator::EncodingMode::Count)) {
        m_state.encodingMode = static_cast<DisplayCodeGenerator::EncodingMode>(storedEncoding);
    } else {
        m_state.encodingMode = PixelFormatCatalog::migrateLegacy(storedEncoding);
    }
    m_state.monoLayout = static_cast<DisplayCodeGenerator::MonoLayout>(qBound(0, snapshot.monoLayout, 2));
    m_state.rotation = snapshot.rotation;
    m_state.flipHorizontal = snapshot.flipHorizontal;
    m_state.flipVertical = snapshot.flipVertical;
    m_state.invertMono = snapshot.invertMono;
    m_state.showGrid = snapshot.showGrid;
    m_state.gridThresholdZoom = snapshot.gridThresholdZoom;
    m_state.filterParams = snapshot.filterParams;
    m_state.filterParams.threshold = m_state.monoThreshold;
    m_state.codeGenOptions.includeHeaderComments = snapshot.codeIncludeComments;
    m_state.codeGenOptions.useProgmem = snapshot.codeUseProgmem;
    m_state.codeGenOptions.staticStorage = snapshot.codeStaticStorage;
    m_state.codeGenOptions.rgb565BigEndian = snapshot.rgb565BigEndian;
    m_state.codeGenOptions.dmaPaddingAlign = qBound(0, snapshot.codeDmaAlign, 8);
    if (m_state.codeGenOptions.dmaPaddingAlign != 4 && m_state.codeGenOptions.dmaPaddingAlign != 8)
        m_state.codeGenOptions.dmaPaddingAlign = 0;
    m_state.linearColorSpace = snapshot.linearColorSpace;
    if (m_state.filterParams.ditherMode == ImageFiltersPipeline::DitherMode::FloydSteinberg)
        m_state.dithering = true;
    else if (m_state.filterParams.ditherMode == ImageFiltersPipeline::DitherMode::None)
        m_state.dithering = false;
    m_displayOutput.syncProfileFromDimensions();
    m_state.colorMode = ConverterEncoding::isColorMode(static_cast<int>(m_state.encodingMode))
        ? DisplayProfile::Rgb565
        : DisplayProfile::Mono1Bit;
    notifyAllToolsChanged();
}

void DisplayConverter::schedulePersistSession()
{
    if (m_session)
        m_sessionSaveTimer.start();
}

void DisplayConverter::loadPersistedSession()
{
    if (!m_session)
        return;
    SessionSnapshot snapshot = SessionSettings::defaultSnapshot();
    const bool hadShowGrid = m_session->containsKey(QStringLiteral("showGrid"));
    m_session->load(&snapshot);
    applySessionSnapshot(snapshot);
    if (!hadShowGrid && m_appSettings)
        m_viewport.setShowGrid(m_appSettings->showPixelGrid());
}

void DisplayConverter::persistSession()
{
    if (m_session)
        m_session->save(sessionSnapshot());
    persistUiState();
}

void DisplayConverter::persistUiState()
{
    if (!m_session)
        return;
    if (!m_state.projectFile.isEmpty())
        m_state.uiState.lastProjectFile = m_state.projectFile.toLocalFile();
    m_session->saveUiState(m_state.uiState);
    m_project.notifyUiFoldersChanged();
    m_export.notifyUiFoldersChanged();
}

void DisplayConverter::refreshLocalization()
{
    ++m_state.localizationRevision;
    emit localizationRevisionChanged();
    m_displayOutput.notifyAllChanged();
}

void DisplayConverter::rememberOpenSourceInRecent()
{
    if (!m_session || !hasImage() || m_state.sourceFilePath.isEmpty())
        return;
    if (!QFileInfo::exists(m_state.sourceFilePath))
        return;
    m_session->addRecentFile(QUrl::fromLocalFile(m_state.sourceFilePath));
}

void DisplayConverter::flushPersistence()
{
    m_sessionSaveTimer.stop();
    persistSession();
}

void DisplayConverter::applyDefaultGridPreference()
{
    if (!m_appSettings)
        return;
    m_viewport.setShowGrid(m_appSettings->showPixelGrid());
}

void DisplayConverter::reloadImportedSettings()
{
    if (!m_session)
        return;

    if (m_appSettings)
        m_appSettings->reloadFromDisk();

    const bool hadShowGrid = m_session->containsKey(QStringLiteral("showGrid"));
    SessionSnapshot snapshot = SessionSettings::defaultSnapshot();
    m_session->load(&snapshot);
    applySessionSnapshot(snapshot);
    if (!hadShowGrid && m_appSettings)
        m_viewport.setShowGrid(m_appSettings->showPixelGrid());

    m_session->loadUiState(&m_state.uiState);
    m_session->pruneMissingRecentFiles();
    m_session->pruneMissingRecentExports();
    m_export.applyStoredWatchState();
    updateWatchExportPrefix();
    restartAutosaveTimer();
    m_project.notifyRecentFilesChanged();
    m_export.notifyRecentExportsChanged();
    m_project.notifyUiFoldersChanged();
    m_export.notifyUiFoldersChanged();
}

void DisplayConverter::updateWatchExportPrefix()
{
    QString stem = DisplayCodeGenerator::sanitizeIdentifier(m_state.project.name);
    if (stem.isEmpty())
        stem = QStringLiteral("project");
    m_export.setExportNamePrefix(stem);
}

void DisplayConverter::restartAutosaveTimer()
{
    m_autosaveTimer.stop();
    if (!m_appSettings || !m_appSettings->projectAutosave())
        return;
    m_autosaveTimer.setInterval(qMax(30, m_appSettings->projectAutosaveSeconds()) * 1000);
    m_autosaveTimer.start();
}

void DisplayConverter::onAutosaveTimeout()
{
    if (!m_appSettings || !m_appSettings->projectAutosave())
        return;
    if (m_state.projectFile.isEmpty() || !hasImage())
        return;
    m_project.saveProject();
}
