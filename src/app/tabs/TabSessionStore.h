#ifndef PIXELSTUDIO_APP_TABS_TABSESSIONSTORE_H
#define PIXELSTUDIO_APP_TABS_TABSESSIONSTORE_H

#include <QList>
#include <QSettings>
#include <QString>

#include "app/tabs/TabTypes.h"

class TabSessionStore
{
public:
    explicit TabSessionStore(const QString &settingsPath);

    void load(QList<StudioTabEntry> *tabs, QString *activeTabId) const;
    void save(const QList<StudioTabEntry> &tabs, const QString &activeTabId);
    bool wasCleanExit() const;
    void markDirtyExit();
    void markCleanExit();

private:
    QSettings m_settings;
};

#endif // PIXELSTUDIO_APP_TABS_TABSESSIONSTORE_H
