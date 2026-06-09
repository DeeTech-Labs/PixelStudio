#include "app/tabs/TabPreloadService.h"

#include "app/preview/PreviewImageProvider.h"
#include "app/studio/TabStateService.h"
#include "app/studio/model/ConverterTabSnapshot.h"
#include "app/tabs/TabTypes.h"
#include "persistence/ProjectFormat.h"
#include "persistence/ProjectService.h"

#include <QFileInfo>

namespace {

QString diskPathForTab(const StudioTabEntry &tab)
{
    if (!tab.cachePath.isEmpty() && QFileInfo::exists(tab.cachePath))
        return tab.cachePath;
    if (!tab.projectPath.isEmpty() && QFileInfo::exists(tab.projectPath))
        return tab.projectPath;
    return {};
}

} // namespace

void TabPreloadService::warmAll(QList<StudioTabEntry> *tabs, PreviewImageProvider *provider)
{
    if (!tabs || !provider)
        return;

    for (StudioTabEntry &tab : *tabs) {
        if (tab.isWelcome || tab.tabSnapshot.has_value())
            continue;

        const QString path = diskPathForTab(tab);
        if (path.isEmpty())
            continue;

        StudioProject project;
        if (!ProjectService::load(QUrl::fromLocalFile(path), &project, nullptr))
            continue;
        if (!ProjectService::hasImageContent(project))
            continue;

        ConverterTabSnapshot snapshot;
        snapshot.tabId = tab.id;
        snapshot.project = project;
        snapshot.projectFileUrl = QUrl::fromLocalFile(path);

        for (const ProjectAsset &asset : project.assets) {
            if (!QFileInfo::exists(asset.path))
                continue;
            snapshot.sourceFilePath = QFileInfo(asset.path).absoluteFilePath();
            if (snapshot.sourceImage.load(asset.path))
                break;
        }
        if (snapshot.sourceImage.isNull() && !project.sourceImagePng.isEmpty())
            snapshot.sourceImage.loadFromData(project.sourceImagePng, "PNG");

        if (!project.resultPreviewPng.isEmpty()) {
            QImage preview;
            if (preview.loadFromData(project.resultPreviewPng, "PNG")) {
                snapshot.lastResult.preview = preview;
                snapshot.hasPipelineResult = true;
            }
        }

        TabStateService::publishSnapshotPreviews(provider, &snapshot);
        tab.tabSnapshot = snapshot;
    }
}
