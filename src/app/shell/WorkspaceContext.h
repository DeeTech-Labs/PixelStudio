#ifndef PIXELSTUDIO_APP_SHELL_WORKSPACECONTEXT_H
#define PIXELSTUDIO_APP_SHELL_WORKSPACECONTEXT_H

#include <QObject>

class DisplayConverter;
class ImageFilterController;
class ImageTransformController;
class DisplayOutputController;
class CodeGenController;
class ViewportController;
class ProjectController;
class ExportController;

#include "app/tabs/StudioTabController.h"
#include "persistence/AppSettings.h"

class WorkspaceContext : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QObject *image READ image CONSTANT)
    Q_PROPERTY(QObject *filters READ filters CONSTANT)
    Q_PROPERTY(QObject *transform READ transform CONSTANT)
    Q_PROPERTY(QObject *output READ output CONSTANT)
    Q_PROPERTY(QObject *code READ code CONSTANT)
    Q_PROPERTY(QObject *viewport READ viewport CONSTANT)
    Q_PROPERTY(QObject *project READ project CONSTANT)
    Q_PROPERTY(QObject *exportPanel READ exportPanel CONSTANT)
    Q_PROPERTY(StudioTabController *tabs READ tabs CONSTANT)
    Q_PROPERTY(AppSettings *settings READ settings CONSTANT)

public:
    explicit WorkspaceContext(DisplayConverter *converter,
                              StudioTabController *tabs,
                              AppSettings *settings,
                              QObject *parent = nullptr);

    QObject *image() const;
    QObject *filters() const;
    QObject *transform() const;
    QObject *output() const;
    QObject *code() const;
    QObject *viewport() const;
    QObject *project() const;
    QObject *exportPanel() const;
    StudioTabController *tabs() const { return m_tabs; }
    AppSettings *settings() const { return m_settings; }

private:
    DisplayConverter *m_converter = nullptr;
    StudioTabController *m_tabs = nullptr;
    AppSettings *m_settings = nullptr;
};

#endif // PIXELSTUDIO_APP_SHELL_WORKSPACECONTEXT_H
