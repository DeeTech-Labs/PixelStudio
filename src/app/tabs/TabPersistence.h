#ifndef PIXELSTUDIO_APP_TABS_TABPERSISTENCE_H
#define PIXELSTUDIO_APP_TABS_TABPERSISTENCE_H

#include <QByteArray>
#include <QImage>
#include <QList>
#include <QString>

#include "app/tabs/TabTypes.h"

class TabPersistence
{
public:
    static QByteArray imageToPng(const QImage &image);
    static void slimInactiveSnapshot(ConverterTabSnapshot *snapshot, const QString &cachePath);
    static void flushCachesToDisk(QList<StudioTabEntry> &tabs, const QString &activeTabId);
    static void removeCacheFile(const QString &cachePath);
};

#endif // PIXELSTUDIO_APP_TABS_TABPERSISTENCE_H
