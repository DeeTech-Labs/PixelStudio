#ifndef PIXELSTUDIO_APP_STUDIO_SESSIONPERSISTENCESERVICE_H
#define PIXELSTUDIO_APP_STUDIO_SESSIONPERSISTENCESERVICE_H

#include <QObject>
#include <QTimer>

class AppSettings;
class DisplayConverter;
class SessionSettings;

class SessionPersistenceService : public QObject
{
    Q_OBJECT

public:
    SessionPersistenceService(SessionSettings *session, AppSettings *appSettings, QObject *parent = nullptr);

    void bind(DisplayConverter *converter);

    void schedulePersist();
    void flush();
    void loadInitial();
    void reloadImported();
    void restartAutosave();

private:
    void persistSession();
    void persistUiState();
    void onAutosaveTimeout();

    DisplayConverter *m_converter = nullptr;
    SessionSettings *m_session = nullptr;
    AppSettings *m_appSettings = nullptr;
    QTimer m_sessionSaveTimer;
    QTimer m_autosaveTimer;
};

#endif // PIXELSTUDIO_APP_STUDIO_SESSIONPERSISTENCESERVICE_H
