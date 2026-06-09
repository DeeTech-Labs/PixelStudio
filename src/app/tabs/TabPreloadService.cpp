#include "app/tabs/TabPreloadService.h"

#include "app/preview/PreviewImageProvider.h"
#include "app/studio/TabStateService.h"
#include "app/tabs/TabTypes.h"
#include "persistence/ProjectFormat.h"
#include "persistence/ProjectService.h"

#include <QtConcurrent>

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

ConverterTabSnapshot loadSnapshotForTab(const StudioTabEntry &tab)
{
    ConverterTabSnapshot snapshot;
    snapshot.tabId = tab.id;

    const QString path = diskPathForTab(tab);
    if (path.isEmpty())
        return snapshot;

    StudioProject project;
    if (!ProjectService::load(QUrl::fromLocalFile(path), &project, nullptr))
        return snapshot;
    if (!ProjectService::hasImageContent(project))
        return snapshot;

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

    return snapshot;
}

} // namespace

TabPreloadService::TabPreloadService(QObject *parent)
    : QObject(parent)
{
    connect(&m_watcher, &QFutureWatcher<QList<TabPreloadResult>>::finished, this, [this]() {
        applyWarmResults(m_watcher.result());
        emit warmFinished();
    });
}

void TabPreloadService::startWarm(QList<StudioTabEntry> *tabs, PreviewImageProvider *provider)
{
    if (!tabs || !provider || tabs->isEmpty())
        return;

    if (m_watcher.isRunning())
        m_watcher.waitForFinished();

    m_tabs = tabs;
    m_provider = provider;

    const QList<StudioTabEntry> tabsCopy = *tabs;
    m_watcher.setFuture(QtConcurrent::run([tabsCopy]() {
        QList<TabPreloadResult> results;
        results.reserve(tabsCopy.size());
        for (const StudioTabEntry &tab : tabsCopy) {
            if (tab.isWelcome || tab.tabSnapshot.has_value())
                continue;
            ConverterTabSnapshot snapshot = loadSnapshotForTab(tab);
            if (snapshot.sourceImage.isNull() && !snapshot.hasPipelineResult)
                continue;
            results.append({tab.id, snapshot});
        }
        return results;
    }));
}

bool TabPreloadService::isRunning() const
{
    return m_watcher.isRunning();
}

void TabPreloadService::applyWarmResults(const QList<TabPreloadResult> &results)
{
    if (!m_tabs || !m_provider)
        return;

    for (const TabPreloadResult &result : results) {
        for (StudioTabEntry &tab : *m_tabs) {
            if (tab.id != result.tabId || tab.tabSnapshot.has_value())
                continue;
            ConverterTabSnapshot snapshot = result.snapshot;
            TabStateService::publishSnapshotPreviews(m_provider, &snapshot);
            tab.tabSnapshot = snapshot;
            break;
        }
    }

    m_tabs = nullptr;
    m_provider = nullptr;
}
