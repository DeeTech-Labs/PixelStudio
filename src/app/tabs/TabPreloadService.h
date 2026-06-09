#ifndef PIXELSTUDIO_APP_TABS_TABPRELOADSERVICE_H
#define PIXELSTUDIO_APP_TABS_TABPRELOADSERVICE_H

#include <QList>

class PreviewImageProvider;
struct StudioTabEntry;

class TabPreloadService
{
public:
    static void warmAll(QList<StudioTabEntry> *tabs, PreviewImageProvider *provider);
};

#endif // PIXELSTUDIO_APP_TABS_TABPRELOADSERVICE_H
