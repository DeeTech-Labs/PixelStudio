#include "app/tabs/TabSessionStore.h"

#include "persistence/StoredPath.h"

namespace {

constexpr auto kTabsKey = "app/tabs";
constexpr auto kActiveTabKey = "app/activeTabId";
constexpr auto kCleanExitKey = "app/cleanExit";

} // namespace

TabSessionStore::TabSessionStore(const QString &settingsPath)
    : m_settings(settingsPath, QSettings::IniFormat)
{
}

void TabSessionStore::load(QList<StudioTabEntry> *tabs, QString *activeTabId) const
{
    if (!tabs || !activeTabId)
        return;

    tabs->clear();
    const QVariantList stored = m_settings.value(QLatin1String(kTabsKey)).toList();
    for (const QVariant &value : stored) {
        const QVariantMap map = value.toMap();
        StudioTabEntry entry;
        entry.id = map.value(QStringLiteral("id")).toString();
        if (entry.id.isEmpty())
            continue;
        entry.title = map.value(QStringLiteral("title")).toString();
        entry.pinned = map.value(QStringLiteral("pinned")).toBool();
        entry.isWelcome = map.value(QStringLiteral("isWelcome")).toBool();
        entry.projectPath = StoredPath::decode(map.value(QStringLiteral("projectPath")).toString());
        entry.cachePath = StoredPath::decode(map.value(QStringLiteral("cachePath")).toString());
        entry.viewMode = map.value(QStringLiteral("viewMode"), 0).toInt();
        tabs->append(entry);
    }
    *activeTabId = m_settings.value(QLatin1String(kActiveTabKey)).toString();
}

void TabSessionStore::save(const QList<StudioTabEntry> &tabs, const QString &activeTabId)
{
    QVariantList stored;
    stored.reserve(tabs.size());
    for (const StudioTabEntry &tab : tabs) {
        stored.append(QVariantMap{
            {QStringLiteral("id"), tab.id},
            {QStringLiteral("title"), tab.title},
            {QStringLiteral("pinned"), tab.pinned},
            {QStringLiteral("isWelcome"), tab.isWelcome},
            {QStringLiteral("projectPath"), StoredPath::encode(tab.projectPath)},
            {QStringLiteral("cachePath"), StoredPath::encode(tab.cachePath)},
            {QStringLiteral("viewMode"), tab.viewMode},
        });
    }
    m_settings.setValue(QLatin1String(kTabsKey), stored);
    m_settings.setValue(QLatin1String(kActiveTabKey), activeTabId);
    m_settings.sync();
}

bool TabSessionStore::wasCleanExit() const
{
    return m_settings.value(QLatin1String(kCleanExitKey), true).toBool();
}

void TabSessionStore::markDirtyExit()
{
    m_settings.setValue(QLatin1String(kCleanExitKey), false);
    m_settings.sync();
}

void TabSessionStore::markCleanExit()
{
    m_settings.setValue(QLatin1String(kCleanExitKey), true);
    m_settings.sync();
}
