#ifndef PIXELSTUDIO_APP_TABS_TABWELCOMESERVICE_H
#define PIXELSTUDIO_APP_TABS_TABWELCOMESERVICE_H

#include <QVariantList>

#include "app/tabs/TabTypes.h"

class DisplayConverter;

class TabWelcomeService
{
public:
    static QVariantMap recentRowForTab(const StudioTabEntry &tab);
    static QVariantList mergedRecentItems(DisplayConverter *converter,
                                          const QList<StudioTabEntry> &tabs);
};

#endif // PIXELSTUDIO_APP_TABS_TABWELCOMESERVICE_H
