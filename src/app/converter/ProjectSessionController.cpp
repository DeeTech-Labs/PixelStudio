#include "app/converter/ProjectSessionController.h"
#include "app/converter/ImagePipelineController.h"
#include "app/DisplayConverter.h"

#include "i18n/AppLocale.h"
#include "io/ImageLoader.h"
#include "persistence/AppPaths.h"
#include "persistence/ProjectService.h"
#include "persistence/SessionSettings.h"

#include <QBuffer>
#include <QDir>
#include <QFileInfo>

StudioProject ProjectSessionController::projectSnapshot(const DisplayConverter &converter)
{
    StudioProject project = ProjectService::fromSession(converter.m_project.name,
                                                        converter.sessionSnapshot(),
                                                        converter.m_offsetX,
                                                        converter.m_offsetY);
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

    const bool hasSourceFile = !converter.m_sourceFilePath.isEmpty()
        && QFileInfo::exists(converter.m_sourceFilePath);
    if (hasSourceFile) {
        project.assets.append(ProjectAsset{
            converter.m_sourceFilePath,
            QFileInfo(converter.m_sourceFilePath).fileName(),
            0,
            0,
        });
    } else if (!converter.m_sourceImage.isNull()) {
        project.sourceImagePng = savePng(converter.m_sourceImage);
    }

    project.resultPreviewPng = savePng(converter.m_lastResult.preview);
    return project;
}

void ProjectSessionController::applyProject(DisplayConverter &converter, const StudioProject &project)
{
    converter.m_project = project;
    converter.m_offsetX = project.offsetX;
    converter.m_offsetY = project.offsetY;
    converter.m_sourceFilePath.clear();
    converter.applySessionSnapshot(project.session);

    QImage image;
    for (const ProjectAsset &asset : project.assets) {
        if (!QFileInfo::exists(asset.path))
            continue;
        if (converter.m_loader->loadFromFile(QUrl::fromLocalFile(asset.path), image)) {
            converter.m_sourceFilePath = QFileInfo(asset.path).absoluteFilePath();
            break;
        }
    }
    if (image.isNull() && !project.sourceImagePng.isEmpty())
        image.loadFromData(project.sourceImagePng, "PNG");

    if (!image.isNull()) {
        converter.m_sourceImage = image;
        converter.markOrientedDirty();
        converter.refreshSourcePreview();
        emit converter.hasImageChanged();
        emit converter.sourceWidthChanged();
        emit converter.sourceHeightChanged();
        ImagePipelineController::scheduleRebuild(converter, true);
    } else {
        converter.m_sourceImage = QImage();
        converter.m_sourcePath.clear();
        emit converter.hasImageChanged();
        emit converter.sourcePathChanged();
        ImagePipelineController::scheduleRebuild(converter, true);
    }

    emit converter.offsetChanged();
    converter.updateWatchExportPrefix();
    emit converter.projectChanged();
}

bool ProjectSessionController::openProject(DisplayConverter &converter, const QUrl &url)
{
    StudioProject project;
    QString error;
    if (!ProjectService::load(url, &project, &error)) {
        emit converter.errorOccurred(error);
        return false;
    }
    applyProject(converter, project);
    converter.m_projectFile = url;
    const QString localPath = url.toLocalFile();
    if (!AppPaths::isExcludedFromRecentPath(localPath) && converter.m_session
        && ProjectService::hasImageContent(project)) {
        converter.m_session->addRecentFile(url);
    }
    if (!AppPaths::isInternalDataPath(localPath) && ProjectService::hasImageContent(project))
        converter.m_uiState.lastProjectFile = localPath;
    converter.updateWatchExportPrefix();
    converter.restartAutosaveTimer();
    converter.schedulePersistSession();
    emit converter.projectChanged();
    return true;
}

bool ProjectSessionController::saveProject(DisplayConverter &converter)
{
    if (converter.m_projectFile.isEmpty()) {
        emit converter.errorOccurred(AppLocale::tr("Specify a project path"));
        return false;
    }
    return saveProjectAs(converter, converter.m_projectFile);
}

bool ProjectSessionController::saveProjectAs(DisplayConverter &converter, const QUrl &url)
{
    const QString previousPath = converter.m_projectFile.toLocalFile();
    converter.m_project = projectSnapshot(converter);
    QString error;
    if (!ProjectService::save(converter.m_project, url, &error)) {
        emit converter.errorOccurred(error);
        return false;
    }
    converter.m_projectFile = url;
    const QString localPath = url.toLocalFile();
    if (!AppPaths::isExcludedFromRecentPath(localPath) && converter.m_session
        && ProjectService::hasImageContent(converter.m_project)) {
        converter.m_session->addRecentFile(url);
    }
    if (!previousPath.isEmpty() && AppPaths::isTabCachePath(previousPath)
        && !AppPaths::isTabCachePath(localPath) && converter.m_session) {
        converter.m_session->removeRecentPath(previousPath);
    }
    if (!AppPaths::isInternalDataPath(localPath) && ProjectService::hasImageContent(converter.m_project))
        converter.m_uiState.lastProjectFile = localPath;
    converter.schedulePersistSession();
    converter.updateWatchExportPrefix();
    converter.restartAutosaveTimer();
    emit converter.projectChanged();
    return true;
}

void ProjectSessionController::newProject(DisplayConverter &converter, const QString &name)
{
    const SessionSnapshot defaults = SessionSettings::defaultSnapshot();
    converter.applySessionSnapshot(defaults);
    converter.m_offsetX = 0;
    converter.m_offsetY = 0;
    emit converter.offsetChanged();
    converter.m_project = ProjectService::fromSession(name, defaults, 0, 0);
    converter.m_projectFile = QUrl();
    converter.m_generatedCode.clear();
    converter.m_previewPath.clear();
    converter.m_processPreviewPath.clear();
    converter.m_lastResult = {};
    ImagePipelineController::updateCodePreview(converter);
    emit converter.previewPathChanged();
    emit converter.processPreviewPathChanged();
    emit converter.generatedCodeChanged();
    converter.updateWatchExportPrefix();
    converter.restartAutosaveTimer();
    emit converter.projectChanged();
}

void ProjectSessionController::setProjectFileUrl(DisplayConverter &converter, const QUrl &url)
{
    converter.m_projectFile = url;
    emit converter.projectChanged();
}

void ProjectSessionController::clearProjectFileUrl(DisplayConverter &converter)
{
    converter.m_projectFile = QUrl();
    emit converter.projectChanged();
}
