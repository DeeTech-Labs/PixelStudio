#include "app/studio/controllers/project/ProjectController.h"
#include "app/studio/pipeline/ImagePipelineController.h"
#include "app/studio/DisplayConverter.h"

#include "translation/AppLocale.h"
#include "io/HeaderImportService.h"
#include "io/ImageLoader.h"
#include "persistence/AppPaths.h"
#include "persistence/ProjectService.h"
#include "persistence/SessionSettings.h"

#include <QBuffer>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileInfo>

ProjectController::ProjectController(QObject *parent)
    : QObject(parent)
{
}

void ProjectController::attach(DisplayConverter *host, ConverterState *state, SessionSettings *session)
{
    m_host = host;
    m_state = state;
    m_session = session;
    if (m_session) {
        connect(m_session, &SessionSettings::recentFilesChanged, this, &ProjectController::recentFilesChanged);
    }
}

QString ProjectController::projectName() const
{
    return m_state ? m_state->project.name : QString{};
}

QUrl ProjectController::projectFile() const
{
    return m_state ? m_state->projectFile : QUrl{};
}

QVariantList ProjectController::projectAssets() const
{
    return m_state ? ProjectService::assetsToVariantList(m_state->project.assets) : QVariantList{};
}

QVariantList ProjectController::recentFiles() const
{
    return m_session ? m_session->recentFiles() : QVariantList{};
}

QString ProjectController::lastProjectPath() const
{
    if (!m_state)
        return {};
    if (AppPaths::isInternalDataPath(m_state->uiState.lastProjectFile))
        return {};
    return m_state->uiState.lastProjectFile;
}

bool ProjectController::hasRestorableProject() const
{
    const QString path = lastProjectPath();
    return !path.isEmpty() && QFileInfo::exists(path);
}

QString ProjectController::lastOpenImageDir() const
{
    if (m_state && !m_state->uiState.lastOpenImageDir.isEmpty())
        return m_state->uiState.lastOpenImageDir;
    return AppPaths::userDocumentsRoot();
}

StudioProject ProjectController::projectSnapshotForDisk() const
{
    if (!m_host || !m_state)
        return {};

    StudioProject project = ProjectService::fromSession(m_state->project.name,
                                                        m_host->sessionSnapshot(),
                                                        m_state->offsetX,
                                                        m_state->offsetY);
    project.assets.clear();
    project.sourceImagePng.clear();
    project.resultPreviewPng.clear();

    auto savePng = [](const QImage &image) -> QByteArray {
        if (image.isNull())
            return {};
        QByteArray png;
        QBuffer buffer(&png);
        buffer.open(QIODevice::WriteOnly);
        return image.save(&buffer, "PNG") ? png : QByteArray{};
    };

    const bool hasSourceFile = !m_state->sourceFilePath.isEmpty()
        && QFileInfo::exists(m_state->sourceFilePath);
    if (hasSourceFile) {
        project.assets.append(ProjectAsset{
            m_state->sourceFilePath,
            QFileInfo(m_state->sourceFilePath).fileName(),
            0,
            0,
        });
    } else if (!m_state->sourceImage.isNull()) {
        project.sourceImagePng = savePng(m_state->sourceImage);
    }

    project.resultPreviewPng = savePng(m_state->lastResult.preview);
    return project;
}

void ProjectController::applyProject(const StudioProject &project)
{
    if (!m_host || !m_state)
        return;

    m_state->project = project;
    m_state->offsetX = project.offsetX;
    m_state->offsetY = project.offsetY;
    m_state->sourceFilePath.clear();
    m_host->applySessionSnapshot(project.session);

    QImage image;
    for (const ProjectAsset &asset : project.assets) {
        if (!QFileInfo::exists(asset.path))
            continue;
        if (m_host->loader()->loadFromFile(QUrl::fromLocalFile(asset.path), image)) {
            m_state->sourceFilePath = QFileInfo(asset.path).absoluteFilePath();
            break;
        }
    }
    if (image.isNull() && !project.sourceImagePng.isEmpty())
        image.loadFromData(project.sourceImagePng, "PNG");

    if (!image.isNull()) {
        m_state->sourceImage = image;
        m_host->markOrientedDirty();
        m_host->refreshSourcePreview();
        emit m_host->hasImageChanged();
        emit m_host->sourceWidthChanged();
        emit m_host->sourceHeightChanged();
        ImagePipelineController::scheduleRebuild(*m_host, true);
    } else {
        m_state->sourceImage = QImage();
        m_state->sourcePath.clear();
        emit m_host->hasImageChanged();
        emit m_host->sourcePathChanged();
        ImagePipelineController::scheduleRebuild(*m_host, true);
    }

    m_host->imageTransform()->notifyOffsetChanged();
    m_host->updateWatchExportPrefix();
    emit projectChanged();
}

bool ProjectController::openProject(const QUrl &url)
{
    if (!m_host || !m_state)
        return false;

    StudioProject project;
    QString error;
    if (!ProjectService::load(url, &project, &error)) {
        emit errorOccurred(error);
        return false;
    }
    applyProject(project);
    m_state->projectFile = url;
    const QString localPath = url.toLocalFile();
    if (!AppPaths::isExcludedFromRecentPath(localPath) && m_session
        && ProjectService::hasImageContent(project)) {
        m_session->addRecentFile(url);
    }
    if (!AppPaths::isInternalDataPath(localPath) && ProjectService::hasImageContent(project))
        m_state->uiState.lastProjectFile = localPath;
    m_host->updateWatchExportPrefix();
    m_host->restartAutosaveTimer();
    m_host->schedulePersistSession();
    emit projectChanged();
    return true;
}

bool ProjectController::saveProject()
{
    if (!m_state || m_state->projectFile.isEmpty()) {
        emit errorOccurred(AppLocale::tr("Specify a project path"));
        return false;
    }
    return saveProjectAs(m_state->projectFile);
}

bool ProjectController::saveProjectAs(const QUrl &url)
{
    if (!m_host || !m_state)
        return false;

    const QString previousPath = m_state->projectFile.toLocalFile();
    m_state->project = projectSnapshotForDisk();
    QString error;
    if (!ProjectService::save(m_state->project, url, &error)) {
        emit errorOccurred(error);
        return false;
    }
    m_state->projectFile = url;
    const QString localPath = url.toLocalFile();
    if (!AppPaths::isExcludedFromRecentPath(localPath) && m_session
        && ProjectService::hasImageContent(m_state->project)) {
        m_session->addRecentFile(url);
    }
    if (!previousPath.isEmpty() && AppPaths::isTabCachePath(previousPath)
        && !AppPaths::isTabCachePath(localPath) && m_session) {
        m_session->removeRecentPath(previousPath);
    }
    if (!AppPaths::isInternalDataPath(localPath) && ProjectService::hasImageContent(m_state->project))
        m_state->uiState.lastProjectFile = localPath;
    m_host->schedulePersistSession();
    m_host->updateWatchExportPrefix();
    m_host->restartAutosaveTimer();
    emit projectChanged();
    return true;
}

void ProjectController::newProject(const QString &name)
{
    if (!m_host || !m_state)
        return;

    const SessionSnapshot defaults = SessionSettings::defaultSnapshot();
    m_host->applySessionSnapshot(defaults);
    m_state->offsetX = 0;
    m_state->offsetY = 0;
    m_host->imageTransform()->notifyOffsetChanged();
    m_state->project = ProjectService::fromSession(name, defaults, 0, 0);
    m_state->projectFile = QUrl();
    m_state->generatedCode.clear();
    m_state->previewPath.clear();
    m_state->processPreviewPath.clear();
    m_state->lastResult = {};
    ImagePipelineController::updateCodePreview(*m_state, *m_host);
    emit m_host->previewPathChanged();
    emit m_host->processPreviewPathChanged();
    emit m_host->generatedCodeChanged();
    m_host->updateWatchExportPrefix();
    m_host->restartAutosaveTimer();
    m_host->applyDefaultGridPreference();
    emit projectChanged();
}

void ProjectController::setProjectFileUrl(const QUrl &url)
{
    if (!m_state)
        return;
    m_state->projectFile = url;
    emit projectChanged();
}

void ProjectController::clearProjectFileUrl()
{
    if (!m_state)
        return;
    m_state->projectFile = QUrl();
    emit projectChanged();
}

bool ProjectController::importHeader(const QUrl &url)
{
    if (!m_host || !m_state)
        return false;

    const HeaderImportResult result = HeaderImportService::importHeader(url);
    if (!result.ok) {
        emit errorOccurred(result.errorMessage);
        return false;
    }
    m_state->sourceImage = result.preview;
    m_state->arrayName = result.arrayName;
    m_state->displayWidth = result.width;
    m_state->displayHeight = result.height;
    m_state->colorMode = result.colorMode;
    m_state->encodingMode = result.encodingMode;
    m_state->profileId = QStringLiteral("custom");
    m_host->markOrientedDirty();
    m_host->refreshSourcePreview();
    emit m_host->hasImageChanged();
    m_host->notifyAllToolsChanged();
    ImagePipelineController::scheduleRebuild(*m_host, true);
    return true;
}

void ProjectController::rememberOpenImageDir(const QString &dir)
{
    if (!m_state || !m_host)
        return;
    const QString path = QFileInfo(dir).isDir() ? dir : QFileInfo(dir).absolutePath();
    if (path.isEmpty())
        return;
    m_state->uiState.lastOpenImageDir = path;
    m_host->schedulePersistSession();
}

void ProjectController::openUserDocumentsFolder()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(AppPaths::userDocumentsRoot()));
}

void ProjectController::openAppDataFolder()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(AppPaths::dataRoot()));
}

void ProjectController::openLogsFolder()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(AppPaths::logsDir()));
}

void ProjectController::flushPersistence()
{
    if (m_host)
        m_host->flushPersistence();
}

void ProjectController::resetSession()
{
    if (!m_host || !m_session || !m_state)
        return;

    m_session->resetToDefaults();
    m_host->applySessionSnapshot(SessionSettings::defaultSnapshot());
    m_state->uiState = SessionUiState{};
    m_state->uiState.watchOutputFolder = AppPaths::watchDir();
    m_host->exportPanel()->watchService()->configure(QString(), m_state->uiState.watchOutputFolder);
    m_host->exportPanel()->watchService()->setActive(false);
    m_host->schedulePersistSession();
    emit recentFilesChanged();
    m_host->exportPanel()->notifyRecentExportsChanged();
    emit uiFoldersChanged();
    m_host->exportPanel()->notifyWatchFolderChanged();
}

bool ProjectController::exportSettingsTo(const QUrl &folderUrl)
{
    flushPersistence();
    QString dir = folderUrl.toLocalFile();
    if (dir.isEmpty())
        dir = folderUrl.path();
    if (dir.isEmpty())
        return false;
    QDir().mkpath(dir);
    const QString appIni = AppPaths::appSettingsFile();
    const QString sessionIni = AppPaths::sessionSettingsFile();
    bool ok = true;
    ok = QFile::copy(appIni, QDir(dir).absoluteFilePath(QStringLiteral("app.ini"))) && ok;
    ok = QFile::copy(sessionIni, QDir(dir).absoluteFilePath(QStringLiteral("session.ini"))) && ok;
    return ok;
}

bool ProjectController::importSettingsFrom(const QUrl &folderUrl)
{
    QString dir = folderUrl.toLocalFile();
    if (dir.isEmpty())
        dir = folderUrl.path();
    if (dir.isEmpty())
        return false;

    const QString srcApp = QDir(dir).absoluteFilePath(QStringLiteral("app.ini"));
    const QString srcSession = QDir(dir).absoluteFilePath(QStringLiteral("session.ini"));
    if (!QFileInfo::exists(srcApp) && !QFileInfo::exists(srcSession)) {
        emit errorOccurred(AppLocale::tr("No app.ini or session.ini in selected folder"));
        return false;
    }

    flushPersistence();
    bool ok = true;
    if (QFileInfo::exists(srcApp))
        ok = QFile::copy(srcApp, AppPaths::appSettingsFile()) && ok;
    if (QFileInfo::exists(srcSession))
        ok = QFile::copy(srcSession, AppPaths::sessionSettingsFile()) && ok;
    if (!ok) {
        emit errorOccurred(AppLocale::tr("Settings import failed"));
        return false;
    }
    m_host->reloadImportedSettings();
    return true;
}

void ProjectController::notifyProjectChanged()
{
    emit projectChanged();
}

void ProjectController::notifyUiFoldersChanged()
{
    emit uiFoldersChanged();
}

void ProjectController::notifyRecentFilesChanged()
{
    emit recentFilesChanged();
}
