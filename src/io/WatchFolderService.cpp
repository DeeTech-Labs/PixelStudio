#include "io/WatchFolderService.h"

#include "processing/DisplayCodeGenerator.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QVariantMap>

WatchFolderService::WatchFolderService(QObject *parent)
    : QObject(parent)
{
    m_debounce.setSingleShot(true);
    m_debounce.setInterval(300);
    connect(&m_debounce, &QTimer::timeout, this, &WatchFolderService::rescan);
    connect(&m_watcher, &QFileSystemWatcher::directoryChanged, this, &WatchFolderService::scheduleRescan);
}

void WatchFolderService::setExportNamePrefix(const QString &prefix)
{
    m_exportNamePrefix = DisplayCodeGenerator::sanitizeIdentifier(prefix);
}

void WatchFolderService::configure(const QString &inputFolder, const QString &outputFolder)
{
    if (!m_watcher.directories().isEmpty())
        m_watcher.removePaths(m_watcher.directories());
    m_inputFolder = inputFolder;
    m_outputFolder = outputFolder;
    if (!m_inputFolder.isEmpty() && QFileInfo::exists(m_inputFolder))
        m_watcher.addPath(m_inputFolder);
    emit foldersChanged();
    scheduleRescan();
}

void WatchFolderService::setActive(bool active)
{
    if (m_active == active)
        return;
    m_active = active;
    emit activeChanged();
    if (m_active)
        scheduleRescan();
}

QVariantList WatchFolderService::pendingFiles() const
{
    QVariantList out;
    for (const QString &file : m_files) {
        QVariantMap item;
        item.insert(QStringLiteral("path"), file);
        item.insert(QStringLiteral("name"), QFileInfo(file).fileName());
        out.append(item);
    }
    return out;
}

void WatchFolderService::scheduleRescan()
{
    if (m_active)
        m_debounce.start();
}

void WatchFolderService::rescan()
{
    if (!m_active || m_inputFolder.isEmpty())
        return;
    QDir dir(m_inputFolder);
    const QStringList names = dir.entryList({QStringLiteral("*.png"),
                                             QStringLiteral("*.jpg"),
                                             QStringLiteral("*.jpeg"),
                                             QStringLiteral("*.bmp"),
                                             QStringLiteral("*.webp")},
                                            QDir::Files,
                                            QDir::Name);
    QStringList files;
    for (const QString &name : names)
        files.append(dir.absoluteFilePath(name));
    if (files == m_files)
        return;
    m_files = files;
    emit filesChanged();
    if (m_files.isEmpty() || m_outputFolder.isEmpty())
        return;

    QVariantList urls;
    for (const QString &file : m_files)
        urls.append(QUrl::fromLocalFile(file));
    const QString stamp = QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_HHmmss"));
    const QString stem = m_exportNamePrefix.isEmpty()
        ? QStringLiteral("watch_export")
        : QStringLiteral("%1_watch").arg(m_exportNamePrefix);
    const QString target = QDir(m_outputFolder).absoluteFilePath(
        QStringLiteral("%1_%2.h").arg(stem, stamp));
    emit exportRequested(urls, QUrl::fromLocalFile(target));
}
