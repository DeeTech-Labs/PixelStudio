#ifndef PIXELSTUDIO_APP_TABS_TABPRELOADSERVICE_H
#define PIXELSTUDIO_APP_TABS_TABPRELOADSERVICE_H

#include <QObject>

#include "app/studio/model/ConverterTabSnapshot.h"

#include <QFutureWatcher>
#include <QList>
#include <QString>

class PreviewImageProvider;
struct StudioTabEntry;

struct TabPreloadResult
{
    QString tabId;
    ConverterTabSnapshot snapshot;
};

class TabPreloadService : public QObject
{
    Q_OBJECT

public:
    explicit TabPreloadService(QObject *parent = nullptr);

    void startWarm(QList<StudioTabEntry> *tabs, PreviewImageProvider *provider);
    bool isRunning() const;

signals:
    void warmFinished();

private:
    void applyWarmResults(const QList<TabPreloadResult> &results);

    QList<StudioTabEntry> *m_tabs = nullptr;
    PreviewImageProvider *m_provider = nullptr;
    QFutureWatcher<QList<TabPreloadResult>> m_watcher;
};

#endif // PIXELSTUDIO_APP_TABS_TABPRELOADSERVICE_H
