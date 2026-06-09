#include "app/tabs/TabPersistence.h"

#include "persistence/AppPaths.h"
#include "persistence/ProjectFormat.h"
#include "persistence/ProjectService.h"
#include "LogCategories.h"

#include <QBuffer>
#include <QFile>
#include <QFileInfo>

QByteArray TabPersistence::imageToPng(const QImage &image)
{
    if (image.isNull())
        return {};
    QByteArray png;
    QBuffer buffer(&png);
    buffer.open(QIODevice::WriteOnly);
    return image.save(&buffer, "PNG") ? png : QByteArray{};
}

void TabPersistence::slimInactiveSnapshot(ConverterTabSnapshot *snapshot, const QString &cachePath)
{
    if (!snapshot)
        return;

    snapshot->generatedCode.clear();
    snapshot->lastResult = {};
    snapshot->hasPipelineResult = false;

    const bool hasSourceFile = !snapshot->sourceFilePath.isEmpty()
        && QFileInfo::exists(snapshot->sourceFilePath);
    if (hasSourceFile) {
        snapshot->sourceImage = {};
        return;
    }

    if (!snapshot->sourceImage.isNull()) {
        if (snapshot->project.sourceImagePng.isEmpty())
            snapshot->project.sourceImagePng = imageToPng(snapshot->sourceImage);
        snapshot->sourceImage = {};
        return;
    }

    if (!cachePath.isEmpty())
        snapshot->project.resultPreviewPng.clear();
}

void TabPersistence::flushCachesToDisk(QList<StudioTabEntry> &tabs, const QString &activeTabId)
{
    for (StudioTabEntry &tab : tabs) {
        if (tab.isWelcome || !tab.tabSnapshot.has_value())
            continue;
        if (tab.tabSnapshot->sourceImage.isNull()
            && !ProjectService::hasImageContent(tab.tabSnapshot->project)) {
            continue;
        }

        if (tab.cachePath.isEmpty()) {
            tab.cachePath = AppPaths::tabCacheDir() + QLatin1Char('/')
                + tab.id + ProjectFormat::extension();
        }

        StudioProject diskProject = tab.tabSnapshot->project;
        diskProject.sourceImagePng.clear();
        diskProject.resultPreviewPng.clear();
        const bool hasSourceFile = !tab.tabSnapshot->sourceFilePath.isEmpty()
            && QFileInfo::exists(tab.tabSnapshot->sourceFilePath);
        if (hasSourceFile) {
            diskProject.assets.clear();
            diskProject.assets.append(ProjectAsset{
                tab.tabSnapshot->sourceFilePath,
                QFileInfo(tab.tabSnapshot->sourceFilePath).fileName(),
                0,
                0,
            });
        } else if (!tab.tabSnapshot->sourceImage.isNull()) {
            diskProject.sourceImagePng = imageToPng(tab.tabSnapshot->sourceImage);
        }
        if (!tab.tabSnapshot->lastResult.preview.isNull())
            diskProject.resultPreviewPng = imageToPng(tab.tabSnapshot->lastResult.preview);

        if (!ProjectService::save(diskProject, QUrl::fromLocalFile(tab.cachePath), nullptr)) {
            qCWarning(lcTabs) << "Failed to flush tab cache:" << tab.cachePath;
            continue;
        }

        if (tab.id != activeTabId)
            slimInactiveSnapshot(&(*tab.tabSnapshot), tab.cachePath);
    }
}

void TabPersistence::removeCacheFile(const QString &cachePath)
{
    if (!cachePath.isEmpty())
        QFile::remove(cachePath);
}
