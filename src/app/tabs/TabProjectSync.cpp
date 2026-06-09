#include "app/tabs/TabProjectSync.h"

#include "app/studio/DisplayConverter.h"
#include "app/studio/controllers/project/ProjectController.h"
#include "app/tabs/TabTypes.h"
#include "persistence/AppPaths.h"
#include "translation/AppLocale.h"

#include <QFileInfo>

void TabProjectSync::stashProjectPaths(DisplayConverter *converter, StudioTabEntry *active)
{
    if (!converter || !active)
        return;

    if (!converter->project()->projectFile().isEmpty()) {
        const QString path = converter->project()->projectFile().toLocalFile();
        active->projectPath = path;
        if (AppPaths::isTabCachePath(path)) {
            if (active->cachePath.isEmpty())
                active->cachePath = path;
        } else {
            active->cachePath.clear();
            converter->project()->saveProject();
        }
    }
}

QString TabProjectSync::displayTitle(const DisplayConverter *converter)
{
    if (!converter)
        return QString();
    if (!converter->project()->projectFile().isEmpty())
        return QFileInfo(converter->project()->projectFile().toLocalFile()).completeBaseName();
    if (!converter->project()->projectName().isEmpty()
        && converter->project()->projectName() != QStringLiteral("Untitled")) {
        return converter->project()->projectName();
    }
    if (converter->hasImage() && !converter->sourceFilePath().isEmpty())
        return QFileInfo(converter->sourceFilePath()).fileName();
    return converter->project()->projectName().isEmpty()
        ? AppLocale::tr("Untitled")
        : converter->project()->projectName();
}
