#ifndef PIXELSTUDIO_APP_TABS_TABRESTORESERVICE_H
#define PIXELSTUDIO_APP_TABS_TABRESTORESERVICE_H

#include <QList>
#include <QString>

#include "app/tabs/TabTypes.h"

class DisplayConverter;

class TabRestoreService
{
public:
    static bool restoreTab(DisplayConverter *converter, const StudioTabEntry &entry);
    static void applyStartupTabPolicy(QList<StudioTabEntry> *tabs,
                                      QString *activeTabId,
                                      bool wasCleanExit);
};

#endif // PIXELSTUDIO_APP_TABS_TABRESTORESERVICE_H
