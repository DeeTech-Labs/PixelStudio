#ifndef PIXELSTUDIO_APP_CONVERTER_PROJECTSESSIONCONTROLLER_H
#define PIXELSTUDIO_APP_CONVERTER_PROJECTSESSIONCONTROLLER_H

#include <QUrl>

#include "persistence/ProjectService.h"

class DisplayConverter;

class ProjectSessionController
{
public:
    static StudioProject projectSnapshotForDisk(const DisplayConverter &converter);
    static void applyProject(DisplayConverter &converter, const StudioProject &project);
    static bool openProject(DisplayConverter &converter, const QUrl &url);
    static bool saveProject(DisplayConverter &converter);
    static bool saveProjectAs(DisplayConverter &converter, const QUrl &url);
    static void newProject(DisplayConverter &converter, const QString &name);
    static void setProjectFileUrl(DisplayConverter &converter, const QUrl &url);
    static void clearProjectFileUrl(DisplayConverter &converter);
};

#endif // PIXELSTUDIO_APP_CONVERTER_PROJECTSESSIONCONTROLLER_H
