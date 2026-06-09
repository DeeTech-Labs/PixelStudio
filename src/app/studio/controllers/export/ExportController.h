#ifndef PIXELSTUDIO_APP_CONVERTER_EXPORTCONTROLLER_H
#define PIXELSTUDIO_APP_CONVERTER_EXPORTCONTROLLER_H

#include <QObject>
#include <QUrl>
#include <QVariantList>

#include "export/BatchExportService.h"
#include "io/WatchFolderService.h"

class DisplayConverter;
class SessionSettings;
struct ConverterState;

class ExportController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool batchRunning READ batchRunning NOTIFY batchRunningChanged)
    Q_PROPERTY(int batchProgress READ batchProgress NOTIFY batchProgressChanged)
    Q_PROPERTY(bool watchFolderActive READ watchFolderActive WRITE setWatchFolderActive NOTIFY watchFolderChanged)
    Q_PROPERTY(QString watchInputFolder READ watchInputFolder NOTIFY watchFolderChanged)
    Q_PROPERTY(QString watchOutputFolder READ watchOutputFolder NOTIFY watchFolderChanged)
    Q_PROPERTY(QVariantList recentExports READ recentExports NOTIFY recentExportsChanged)
    Q_PROPERTY(QString lastExportDir READ lastExportDir NOTIFY uiFoldersChanged)

public:
    explicit ExportController(QObject *parent = nullptr);

    void attach(DisplayConverter *host, ConverterState *state, SessionSettings *session);

    WatchFolderService *watchService() { return &m_watchService; }
    void setExportNamePrefix(const QString &stem);

    bool batchRunning() const;
    int batchProgress() const;
    bool watchFolderActive() const;
    QString watchInputFolder() const;
    QString watchOutputFolder() const;
    QVariantList recentExports() const;
    QString lastExportDir() const;

    Q_INVOKABLE void configureWatchFolder(const QString &inputFolder, const QString &outputFolder);
    Q_INVOKABLE void configureWatchFolders(const QUrl &inputFolder, const QUrl &outputFolder);
    Q_INVOKABLE void setWatchFolderActive(bool active);
    Q_INVOKABLE bool buildSpriteAtlas(const QVariantList &urls,
                                      const QUrl &targetFile,
                                      int frameWidth,
                                      int frameHeight);
    Q_INVOKABLE void copyToClipboard(const QString &text);
    Q_INVOKABLE bool saveCodeToFile(const QUrl &url);
    Q_INVOKABLE bool saveBinaryToFile(const QUrl &url);
    Q_INVOKABLE void enqueueBatchCodeExport(const QVariantList &urls, const QUrl &targetFile);
    Q_INVOKABLE void cancelBatchExport();
    Q_INVOKABLE QString suggestedCodeFilePath() const;
    Q_INVOKABLE QUrl suggestedCodeFileUrl() const;
    Q_INVOKABLE void rememberExportDir(const QString &dir);

    void resetBatchExport();
    void applyStoredWatchState();
    void notifyWatchFolderChanged();
    void notifyUiFoldersChanged();
    void notifyRecentExportsChanged();

signals:
    void batchRunningChanged();
    void batchProgressChanged();
    void watchFolderChanged();
    void recentExportsChanged();
    void uiFoldersChanged();
    void errorOccurred(const QString &message);

private:
    void onBatchFinished(bool ok, const QString &errorMessage);
    void onWatchExportRequested(const QVariantList &files, const QUrl &targetFile);

    DisplayConverter *m_host = nullptr;
    ConverterState *m_state = nullptr;
    SessionSettings *m_session = nullptr;
    BatchExportService m_batchService;
    WatchFolderService m_watchService;
};

#endif // PIXELSTUDIO_APP_CONVERTER_EXPORTCONTROLLER_H
