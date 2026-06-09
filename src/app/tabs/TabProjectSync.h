#ifndef PIXELSTUDIO_APP_TABS_TABPROJECTSYNC_H
#define PIXELSTUDIO_APP_TABS_TABPROJECTSYNC_H

#include <QString>

class DisplayConverter;
struct StudioTabEntry;

class TabProjectSync
{
public:
    static QString displayTitle(const DisplayConverter *converter);
    static void stashProjectPaths(DisplayConverter *converter, StudioTabEntry *active);
};

#endif // PIXELSTUDIO_APP_TABS_TABPROJECTSYNC_H
