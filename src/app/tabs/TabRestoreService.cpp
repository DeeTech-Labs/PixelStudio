#include "app/tabs/TabRestoreService.h"

#include "app/studio/DisplayConverter.h"
#include "app/studio/controllers/project/ProjectController.h"
#include "app/tabs/StudioTabController.h"
#include "app/tabs/TabPersistence.h"
#include "translation/AppLocale.h"
#include "LogCategories.h"

#include <QFileInfo>
#include <QUrl>

bool TabRestoreService::restoreTab(DisplayConverter *converter, const StudioTabEntry &entry)
{
    if (!converter || entry.isWelcome || entry.isSettings)
        return true;

    converter->setActiveTabId(entry.id);

    if (entry.tabSnapshot.has_value()) {
        ConverterTabSnapshot snapshot = *entry.tabSnapshot;
        if (snapshot.tabId.isEmpty())
            snapshot.tabId = entry.id;
        converter->restoreTabState(snapshot);
        if (!entry.projectPath.isEmpty())
            converter->project()->setProjectFileUrl(QUrl::fromLocalFile(entry.projectPath));
        else if (!entry.cachePath.isEmpty())
            converter->project()->setProjectFileUrl(QUrl::fromLocalFile(entry.cachePath));
        else
            converter->project()->clearProjectFileUrl();
        return true;
    }

    if (!entry.projectPath.isEmpty() && QFileInfo::exists(entry.projectPath))
        return converter->project()->openProject(QUrl::fromLocalFile(entry.projectPath));

    if (!entry.cachePath.isEmpty() && QFileInfo::exists(entry.cachePath))
        return converter->project()->openProject(QUrl::fromLocalFile(entry.cachePath));

    converter->clear();
    converter->project()->newProject(entry.title.isEmpty() ? AppLocale::tr("Untitled") : entry.title);
    return true;
}

void TabRestoreService::applyStartupTabPolicy(QList<StudioTabEntry> *tabs,
                                              QString *activeTabId,
                                              bool wasCleanExit)
{
    if (!tabs || !activeTabId)
        return;

    const auto findTab = [&](const QString &id) -> const StudioTabEntry * {
        for (const StudioTabEntry &tab : *tabs) {
            if (tab.id == id)
                return &tab;
        }
        return nullptr;
    };

    QString nextActive = *activeTabId;
    if (wasCleanExit) {
        const StudioTabEntry *active = findTab(nextActive);
        if (active && !active->isWelcome && !active->pinned)
            nextActive = QString::fromLatin1(StudioTabController::kWelcomeTabId);
    }

    QList<StudioTabEntry> kept;
    kept.reserve(tabs->size());
    for (const StudioTabEntry &tab : *tabs) {
        if (tab.isSettings)
            continue;
        if (tab.isWelcome) {
            kept.append(tab);
            continue;
        }
        if (wasCleanExit) {
            if (tab.pinned)
                kept.append(tab);
            else
                TabPersistence::removeCacheFile(tab.cachePath);
        } else {
            StudioTabEntry copy = tab;
            if (!copy.pinned) {
                copy.recovered = true;
                qCInfo(lcTabs) << "Recovered tab after unclean exit:" << copy.id << copy.title;
            }
            kept.append(copy);
        }
    }
    *tabs = kept;

    if (!findTab(nextActive)) {
        nextActive = QString::fromLatin1(StudioTabController::kWelcomeTabId);
        for (const StudioTabEntry &tab : *tabs) {
            if (tab.pinned && !tab.isWelcome) {
                nextActive = tab.id;
                break;
            }
        }
    }
    *activeTabId = nextActive;
}
