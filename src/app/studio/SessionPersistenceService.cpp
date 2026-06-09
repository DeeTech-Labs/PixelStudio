#include "app/studio/SessionPersistenceService.h"

#include "app/studio/DisplayConverter.h"
#include "app/studio/controllers/export/ExportController.h"
#include "app/studio/controllers/output/ViewportController.h"
#include "app/studio/controllers/project/ProjectController.h"
#include "app/studio/model/ConverterState.h"
#include "persistence/AppSettings.h"
#include "persistence/SessionSettings.h"

SessionPersistenceService::SessionPersistenceService(SessionSettings *session,
                                                     AppSettings *appSettings,
                                                     QObject *parent)
    : QObject(parent)
    , m_session(session)
    , m_appSettings(appSettings)
{
    m_sessionSaveTimer.setSingleShot(true);
    m_sessionSaveTimer.setInterval(400);
    connect(&m_sessionSaveTimer, &QTimer::timeout, this, &SessionPersistenceService::persistSession);

    m_autosaveTimer.setSingleShot(false);
    connect(&m_autosaveTimer, &QTimer::timeout, this, &SessionPersistenceService::onAutosaveTimeout);

    if (m_appSettings) {
        connect(m_appSettings, &AppSettings::projectAutosaveChanged, this, &SessionPersistenceService::restartAutosave);
        connect(m_appSettings,
                &AppSettings::projectAutosaveSecondsChanged,
                this,
                &SessionPersistenceService::restartAutosave);
    }
}

void SessionPersistenceService::bind(DisplayConverter *converter)
{
    m_converter = converter;
}

void SessionPersistenceService::schedulePersist()
{
    if (m_session)
        m_sessionSaveTimer.start();
}

void SessionPersistenceService::flush()
{
    m_sessionSaveTimer.stop();
    persistSession();
}

void SessionPersistenceService::loadInitial()
{
    if (!m_converter || !m_session)
        return;

    SessionSnapshot snapshot = SessionSettings::defaultSnapshot();
    const bool hadShowGrid = m_session->containsKey(QStringLiteral("showGrid"));
    m_session->load(&snapshot);
    m_converter->applySessionSnapshot(snapshot);
    if (!hadShowGrid && m_appSettings)
        m_converter->viewport()->setShowGrid(m_appSettings->showPixelGrid());
}

void SessionPersistenceService::reloadImported()
{
    if (!m_converter || !m_session)
        return;

    if (m_appSettings)
        m_appSettings->reloadFromDisk();

    const bool hadShowGrid = m_session->containsKey(QStringLiteral("showGrid"));
    SessionSnapshot snapshot = SessionSettings::defaultSnapshot();
    m_session->load(&snapshot);
    m_converter->applySessionSnapshot(snapshot);
    if (!hadShowGrid && m_appSettings)
        m_converter->viewport()->setShowGrid(m_appSettings->showPixelGrid());

    ConverterState &state = m_converter->converterState();
    m_session->loadUiState(&state.uiState);
    m_session->pruneMissingRecentFiles();
    m_session->pruneMissingRecentExports();
    m_converter->exportPanel()->applyStoredWatchState();
    m_converter->updateWatchExportPrefix();
    restartAutosave();
    m_converter->project()->notifyRecentFilesChanged();
    m_converter->exportPanel()->notifyRecentExportsChanged();
    m_converter->project()->notifyUiFoldersChanged();
    m_converter->exportPanel()->notifyUiFoldersChanged();
}

void SessionPersistenceService::restartAutosave()
{
    m_autosaveTimer.stop();
    if (!m_appSettings || !m_appSettings->projectAutosave())
        return;
    m_autosaveTimer.setInterval(qMax(30, m_appSettings->projectAutosaveSeconds()) * 1000);
    m_autosaveTimer.start();
}

void SessionPersistenceService::persistSession()
{
    if (!m_converter)
        return;
    if (m_session)
        m_session->save(m_converter->sessionSnapshot());
    persistUiState();
}

void SessionPersistenceService::persistUiState()
{
    if (!m_converter || !m_session)
        return;

    ConverterState &state = m_converter->converterState();
    if (!state.projectFile.isEmpty())
        state.uiState.lastProjectFile = state.projectFile.toLocalFile();
    m_session->saveUiState(state.uiState);
    m_converter->project()->notifyUiFoldersChanged();
    m_converter->exportPanel()->notifyUiFoldersChanged();
}

void SessionPersistenceService::onAutosaveTimeout()
{
    if (!m_converter || !m_appSettings || !m_appSettings->projectAutosave())
        return;

    const ConverterState &state = m_converter->converterState();
    if (state.projectFile.isEmpty() || !m_converter->hasImage())
        return;

    m_converter->project()->saveProject();
}
