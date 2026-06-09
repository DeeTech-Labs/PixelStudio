#ifndef PIXELSTUDIO_APP_TABS_TABTYPES_H
#define PIXELSTUDIO_APP_TABS_TABTYPES_H

#include <QString>
#include <optional>

#include "app/studio/model/ConverterTabSnapshot.h"

struct StudioTabEntry
{
    QString id;
    QString title;
    bool pinned = false;
    bool isWelcome = false;
    bool isSettings = false;
    bool recovered = false;
    QString projectPath;
    QString cachePath;
    int viewMode = 0;
    std::optional<ConverterTabSnapshot> tabSnapshot;
};

#endif // PIXELSTUDIO_APP_TABS_TABTYPES_H
