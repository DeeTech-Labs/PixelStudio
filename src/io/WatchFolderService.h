#ifndef PIXELSTUDIO_IO_WATCHFOLDERSERVICE_H
#define PIXELSTUDIO_IO_WATCHFOLDERSERVICE_H

#include <QObject>
#include <QFileSystemWatcher>
#include <QTimer>
#include <QUrl>
#include <QVariantList>

class WatchFolderService : public QObject
{
    Q_OBJECT
public:
    explicit WatchFolderService(QObject *parent = nullptr);

    bool active() const { return m_active; }
    QString inputFolder() const { return m_inputFolder; }
    QString outputFolder() const { return m_outputFolder; }
    QVariantList pendingFiles() const;

    void configure(const QString &inputFolder, const QString &outputFolder);
    void setExportNamePrefix(const QString &prefix);
    void setActive(bool active);

signals:
    void activeChanged();
    void foldersChanged();
    void filesChanged();
    void exportRequested(const QVariantList &files, const QUrl &targetFile);

private:
    void rescan();
    void scheduleRescan();

    QFileSystemWatcher m_watcher;
    QTimer m_debounce;
    bool m_active = false;
    QString m_inputFolder;
    QString m_outputFolder;
    QString m_exportNamePrefix;
    QStringList m_files;
};

#endif // PIXELSTUDIO_IO_WATCHFOLDERSERVICE_H
