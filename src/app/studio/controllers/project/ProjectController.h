#ifndef PIXELSTUDIO_APP_CONVERTER_PROJECTCONTROLLER_H
#define PIXELSTUDIO_APP_CONVERTER_PROJECTCONTROLLER_H

#include <QObject>
#include <QUrl>
#include <QVariantList>

#include "persistence/ProjectService.h"

class DisplayConverter;
class SessionSettings;
struct ConverterState;

class ProjectController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString projectName READ projectName NOTIFY projectChanged)
    Q_PROPERTY(QUrl projectFile READ projectFile NOTIFY projectChanged)
    Q_PROPERTY(QVariantList projectAssets READ projectAssets NOTIFY projectChanged)
    Q_PROPERTY(QVariantList recentFiles READ recentFiles NOTIFY recentFilesChanged)
    Q_PROPERTY(QString lastProjectPath READ lastProjectPath NOTIFY uiFoldersChanged)
    Q_PROPERTY(bool hasRestorableProject READ hasRestorableProject NOTIFY uiFoldersChanged)
    Q_PROPERTY(QString lastOpenImageDir READ lastOpenImageDir NOTIFY uiFoldersChanged)

public:
    explicit ProjectController(QObject *parent = nullptr);

    void attach(DisplayConverter *host, ConverterState *state, SessionSettings *session);

    QString projectName() const;
    QUrl projectFile() const;
    QVariantList projectAssets() const;
    QVariantList recentFiles() const;
    QString lastProjectPath() const;
    bool hasRestorableProject() const;
    QString lastOpenImageDir() const;

    Q_INVOKABLE void newProject(const QString &name);
    Q_INVOKABLE bool openProject(const QUrl &url);
    Q_INVOKABLE bool saveProject();
    Q_INVOKABLE bool saveProjectAs(const QUrl &url);
    Q_INVOKABLE bool importHeader(const QUrl &url);
    Q_INVOKABLE void rememberOpenImageDir(const QString &dir);
    Q_INVOKABLE void resetSession();
    Q_INVOKABLE bool exportSettingsTo(const QUrl &folderUrl);
    Q_INVOKABLE bool importSettingsFrom(const QUrl &folderUrl);
    Q_INVOKABLE void flushPersistence();
    Q_INVOKABLE void openUserDocumentsFolder();
    Q_INVOKABLE void openAppDataFolder();
    Q_INVOKABLE void openLogsFolder();

    void applyProject(const StudioProject &project);
    StudioProject projectSnapshotForDisk() const;
    void setProjectFileUrl(const QUrl &url);
    void clearProjectFileUrl();
    void notifyProjectChanged();
    void notifyUiFoldersChanged();
    void notifyRecentFilesChanged();

signals:
    void projectChanged();
    void recentFilesChanged();
    void uiFoldersChanged();
    void errorOccurred(const QString &message);

private:
    DisplayConverter *m_host = nullptr;
    ConverterState *m_state = nullptr;
    SessionSettings *m_session = nullptr;
};

#endif // PIXELSTUDIO_APP_CONVERTER_PROJECTCONTROLLER_H
