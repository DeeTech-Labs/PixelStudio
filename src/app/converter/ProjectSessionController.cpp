#include "app/converter/ProjectSessionController.h"
#include "app/converter/ImagePipelineController.h"
#include "app/DisplayConverter.h"

#include "translation/AppLocale.h"
#include "io/ImageLoader.h"
#include "persistence/AppPaths.h"
#include "persistence/ProjectService.h"

#include <QBuffer>
#include <QFileInfo>

StudioProject ProjectSessionController::projectSnapshotForDisk(const DisplayConverter &converter)
{
    const ConverterState &state = converter.converterState();
    StudioProject project = ProjectService::fromSession(state.project.name,
                                                        converter.sessionSnapshot(),
                                                        state.offsetX,
                                                        state.offsetY);
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

    const bool hasSourceFile = !state.sourceFilePath.isEmpty()
        && QFileInfo::exists(state.sourceFilePath);
    if (hasSourceFile) {
        project.assets.append(ProjectAsset{
            state.sourceFilePath,
            QFileInfo(state.sourceFilePath).fileName(),
            0,
            0,
        });
    } else if (!state.sourceImage.isNull()) {
        project.sourceImagePng = savePng(state.sourceImage);
    }

    project.resultPreviewPng = savePng(state.lastResult.preview);
    return project;
}

void ProjectSessionController::applyProject(DisplayConverter &converter, const StudioProject &project)
{
    ConverterState &state = converter.converterState();
    state.project = project;
    state.offsetX = project.offsetX;
    state.offsetY = project.offsetY;
    state.sourceFilePath.clear();
    converter.applySessionSnapshot(project.session);

    QImage image;
    for (const ProjectAsset &asset : project.assets) {
        if (!QFileInfo::exists(asset.path))
            continue;
        if (converter.loader()->loadFromFile(QUrl::fromLocalFile(asset.path), image)) {
            state.sourceFilePath = QFileInfo(asset.path).absoluteFilePath();
            break;
        }
    }
    if (image.isNull() && !project.sourceImagePng.isEmpty())
        image.loadFromData(project.sourceImagePng, "PNG");

    if (!image.isNull()) {
        state.sourceImage = image;
        converter.markOrientedDirty();
        converter.refreshSourcePreview();
        emit converter.hasImageChanged();
        emit converter.sourceWidthChanged();
        emit converter.sourceHeightChanged();
        ImagePipelineController::scheduleRebuild(converter, true);
    } else {
        state.sourceImage = QImage();
        state.sourcePath.clear();
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
    converter.converterState().projectFile = url;
    const QString localPath = url.toLocalFile();
    if (!AppPaths::isExcludedFromRecentPath(localPath) && converter.session()
        && ProjectService::hasImageContent(project)) {
        converter.session()->addRecentFile(url);
    }
    if (!AppPaths::isInternalDataPath(localPath) && ProjectService::hasImageContent(project))
        converter.converterState().uiState.lastProjectFile = localPath;
    converter.updateWatchExportPrefix();
    converter.restartAutosaveTimer();
    converter.schedulePersistSession();
    emit converter.projectChanged();
    return true;
}

bool ProjectSessionController::saveProject(DisplayConverter &converter)
{
    if (converter.converterState().projectFile.isEmpty()) {
        emit converter.errorOccurred(AppLocale::tr("Specify a project path"));
        return false;
    }
    return saveProjectAs(converter, converter.converterState().projectFile);
}

bool ProjectSessionController::saveProjectAs(DisplayConverter &converter, const QUrl &url)
{
    ConverterState &state = converter.converterState();
    const QString previousPath = state.projectFile.toLocalFile();
    state.project = projectSnapshotForDisk(converter);
    QString error;
    if (!ProjectService::save(state.project, url, &error)) {
        emit converter.errorOccurred(error);
        return false;
    }
    state.projectFile = url;
    const QString localPath = url.toLocalFile();
    if (!AppPaths::isExcludedFromRecentPath(localPath) && converter.session()
        && ProjectService::hasImageContent(state.project)) {
        converter.session()->addRecentFile(url);
    }
    if (!previousPath.isEmpty() && AppPaths::isTabCachePath(previousPath)
        && !AppPaths::isTabCachePath(localPath) && converter.session()) {
        converter.session()->removeRecentPath(previousPath);
    }
    if (!AppPaths::isInternalDataPath(localPath) && ProjectService::hasImageContent(state.project))
        state.uiState.lastProjectFile = localPath;
    converter.schedulePersistSession();
    converter.updateWatchExportPrefix();
    converter.restartAutosaveTimer();
    emit converter.projectChanged();
    return true;
}

void ProjectSessionController::newProject(DisplayConverter &converter, const QString &name)
{
    ConverterState &state = converter.converterState();
    const SessionSnapshot defaults = SessionSettings::defaultSnapshot();
    converter.applySessionSnapshot(defaults);
    state.offsetX = 0;
    state.offsetY = 0;
    emit converter.offsetChanged();
    state.project = ProjectService::fromSession(name, defaults, 0, 0);
    state.projectFile = QUrl();
    state.generatedCode.clear();
    state.previewPath.clear();
    state.processPreviewPath.clear();
    state.lastResult = {};
    ImagePipelineController::updateCodePreview(state, converter);
    emit converter.previewPathChanged();
    emit converter.processPreviewPathChanged();
    emit converter.generatedCodeChanged();
    converter.updateWatchExportPrefix();
    converter.restartAutosaveTimer();
    emit converter.projectChanged();
}

void ProjectSessionController::setProjectFileUrl(DisplayConverter &converter, const QUrl &url)
{
    converter.converterState().projectFile = url;
    emit converter.projectChanged();
}

void ProjectSessionController::clearProjectFileUrl(DisplayConverter &converter)
{
    converter.converterState().projectFile = QUrl();
    emit converter.projectChanged();
}
