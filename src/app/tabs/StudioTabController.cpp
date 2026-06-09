#include "app/tabs/StudioTabController.h"

#include "app/studio/DisplayConverter.h"
#include "app/studio/controllers/project/ProjectController.h"
#include "app/tabs/TabPersistence.h"
#include "app/tabs/TabProjectSync.h"
#include "app/preview/PreviewImageProvider.h"
#include "app/tabs/TabPersistence.h"
#include "app/tabs/TabRestoreService.h"
#include "app/tabs/TabWelcomeService.h"
#include "LogCategories.h"
#include "translation/AppLocale.h"
#include "persistence/AppPaths.h"
#include "persistence/StoredPath.h"
#include "persistence/ProjectFormat.h"

#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QTimer>
#include <QUuid>

#include <algorithm>

namespace {

bool isProjectPath(const QString &path)
{
    return ProjectFormat::isProjectPath(path);
}

QString tabPathKey(const QString &path)
{
    if (path.isEmpty())
        return {};
    const QString canonical = QFileInfo(path).canonicalFilePath();
    return canonical.isEmpty() ? path : canonical;
}

int normalizeViewMode(int mode)
{
    switch (mode) {
    case 2:
    case 4:
    case 5:
        return 0;
    default:
        break;
    }
    return qBound(0, mode, 3);
}

} // namespace

StudioTabController::StudioTabController(DisplayConverter *converter, QObject *parent)
    : QObject(parent)
    , m_converter(converter)
    , m_preload(this)
    , m_tabStore(AppPaths::appSettingsFile())
{
    connect(&m_preload, &TabPreloadService::warmFinished, this, [this]() {
        saveTabsToSettings();
        emit tabsChanged();
    });
    setConverter(converter);
}

void StudioTabController::setConverter(DisplayConverter *converter)
{
    if (m_converter) {
        disconnect(m_converter, nullptr, this, nullptr);
    }
    m_converter = converter;
    if (m_converter) {
        connect(m_converter->project(), &ProjectController::projectChanged, this, &StudioTabController::onProjectChanged);
        connect(m_converter->project(), &ProjectController::recentFilesChanged, this, [this]() {
            if (activeIsWelcome())
                scheduleWelcomeRecentRefresh();
            else
                invalidateWelcomeRecent();
        });
    }
}

QVariantList StudioTabController::tabsToVariant() const
{
    QVariantList out;
    out.reserve(m_tabs.size());
    for (const StudioTabEntry &t : m_tabs) {
        out.append(QVariantMap{
            {QStringLiteral("id"), t.id},
            {QStringLiteral("title"), formatTabTitle(t)},
            {QStringLiteral("pinned"), t.pinned},
            {QStringLiteral("isWelcome"), t.isWelcome},
            {QStringLiteral("isSettings"), t.isSettings},
            {QStringLiteral("closable"), !t.isWelcome},
            {QStringLiteral("projectPath"), t.projectPath},
        });
    }
    return out;
}

QVariantList StudioTabController::tabs() const
{
    return tabsToVariant();
}

QVariantList StudioTabController::welcomeRecentItems() const
{
    if (m_welcomeRecentDirty) {
        m_welcomeRecentCache = TabWelcomeService::mergedRecentItems(m_converter, m_tabs);
        m_welcomeRecentDirty = false;
    }
    return m_welcomeRecentCache;
}

void StudioTabController::invalidateWelcomeRecent()
{
    m_welcomeRecentDirty = true;
}

void StudioTabController::scheduleWelcomeRecentRefresh()
{
    invalidateWelcomeRecent();
    QTimer::singleShot(0, this, [this]() {
        if (!activeIsWelcome())
            return;
        emit welcomeRecentItemsChanged();
    });
}

bool StudioTabController::activeIsWelcome() const
{
    const StudioTabEntry *t = findTab(m_activeTabId);
    return !t || t->isWelcome;
}

bool StudioTabController::activeIsSettings() const
{
    const StudioTabEntry *t = findTab(m_activeTabId);
    return t && t->isSettings;
}

QString StudioTabController::activeTabTitle() const
{
    const StudioTabEntry *t = findTab(m_activeTabId);
    return t ? t->title : QString();
}

bool StudioTabController::looksLikeUuidTitle(const QString &title)
{
    static const QRegularExpression uuidRe(
        QStringLiteral("^[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}$"),
        QRegularExpression::CaseInsensitiveOption);
    return uuidRe.match(title.trimmed()).hasMatch();
}

QString StudioTabController::sanitizeTabTitle(const QString &title)
{
    const QString stripped = stripRecoveredPrefix(title);
    if (looksLikeUuidTitle(stripped))
        return AppLocale::tr("Untitled");
    return stripped;
}

QString StudioTabController::stripRecoveredPrefix(const QString &title)
{
    static const QStringList knownPrefixes = {
        QStringLiteral("[Recovered]"),
        QString::fromUtf8("[Восстановлено]"),
    };
    for (const QString &prefix : knownPrefixes) {
        if (!title.startsWith(prefix))
            continue;
        QString rest = title.mid(prefix.size());
        if (rest.startsWith(QLatin1Char(' ')))
            rest = rest.mid(1);
        return rest.trimmed();
    }
    const QString localized = AppLocale::tr("[Recovered]");
    if (title.startsWith(localized)) {
        QString rest = title.mid(localized.size());
        if (rest.startsWith(QLatin1Char(' ')))
            rest = rest.mid(1);
        return rest.trimmed();
    }
    return title;
}

QString StudioTabController::formatTabTitle(const StudioTabEntry &entry) const
{
    QString title = entry.title;
    if (title.isEmpty()) {
        if (entry.isWelcome)
            title = AppLocale::tr("Home");
        else if (entry.isSettings)
            title = AppLocale::tr("Settings");
        else
            title = AppLocale::tr("Untitled");
    }
    if (!entry.recovered || entry.isWelcome || entry.isSettings)
        return title;
    const QString prefix = AppLocale::tr("[Recovered]");
    if (title.startsWith(prefix))
        return title;
    return prefix + QLatin1Char(' ') + title;
}

int StudioTabController::activeViewMode() const
{
    const StudioTabEntry *t = findTab(m_activeTabId);
    return t ? normalizeViewMode(t->viewMode) : 0;
}

void StudioTabController::setActiveViewMode(int mode)
{
    StudioTabEntry *t = findTab(m_activeTabId);
    if (!t)
        return;
    const int clamped = normalizeViewMode(mode);
    if (t->viewMode == clamped)
        return;
    t->viewMode = clamped;
    emit activeViewModeChanged();
    if (!t->isWelcome && !t->isSettings)
        saveTabsToSettings();
}

StudioTabEntry *StudioTabController::findTab(const QString &id)
{
    for (StudioTabEntry &t : m_tabs) {
        if (t.id == id)
            return &t;
    }
    return nullptr;
}

const StudioTabEntry *StudioTabController::findTab(const QString &id) const
{
    for (const StudioTabEntry &t : m_tabs) {
        if (t.id == id)
            return &t;
    }
    return nullptr;
}

QString StudioTabController::createTabId()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}


StudioTabEntry &StudioTabController::ensureWelcomeTab()
{
    for (StudioTabEntry &t : m_tabs) {
        if (t.isWelcome) {
            t.title = AppLocale::tr("Home");
            return t;
        }
    }
    StudioTabEntry welcome;
    welcome.id = QString::fromLatin1(kWelcomeTabId);
    welcome.title = AppLocale::tr("Home");
    welcome.isWelcome = true;
    welcome.pinned = true;
    m_tabs.prepend(welcome);
    return m_tabs.first();
}

void StudioTabController::stashTab(const QString &tabId)
{
    if (!m_converter)
        return;
    StudioTabEntry *tab = findTab(tabId);
    if (!tab || tab->isWelcome || tab->isSettings)
        return;

    tab->title = TabProjectSync::displayTitle(m_converter);

    if (!m_converter->hasImage() && m_converter->project()->projectFile().isEmpty()) {
        if (!tab->cachePath.isEmpty()) {
            QFile::remove(tab->cachePath);
            tab->cachePath.clear();
        }
        tab->projectPath.clear();
        tab->tabSnapshot.reset();
        return;
    }

    tab->tabSnapshot = m_converter->captureTabState(tab->id);
    if (tab->id != m_activeTabId)
        TabPersistence::slimInactiveSnapshot(&(*tab->tabSnapshot), tab->cachePath);
    m_converter->rememberOpenSourceInRecent();
    TabProjectSync::stashProjectPaths(m_converter, tab);
    invalidateWelcomeRecent();
}

void StudioTabController::stashActiveTab()
{
    stashTab(m_activeTabId);
}

void StudioTabController::loadTabsFromSettings()
{
    m_tabStore.load(&m_tabs, &m_activeTabId);
    for (qsizetype i = m_tabs.size() - 1; i >= 0; --i) {
        StudioTabEntry &tab = m_tabs[i];
        if (tab.isSettings) {
            m_tabs.removeAt(i);
            continue;
        }
        if (tab.isWelcome)
            tab.id = QString::fromLatin1(kWelcomeTabId);
        tab.title = sanitizeTabTitle(tab.title);
        if (tab.title.isEmpty())
            tab.title = tab.isWelcome ? AppLocale::tr("Home") : AppLocale::tr("Untitled");
        tab.viewMode = normalizeViewMode(tab.viewMode);
    }
    if (m_activeTabId == QString::fromLatin1(kSettingsTabId))
        m_activeTabId.clear();
    ensureWelcomeTab();
    if (!findTab(m_activeTabId))
        m_activeTabId = QString::fromLatin1(kWelcomeTabId);
}

void StudioTabController::saveTabsToSettings()
{
    QString activeId = m_activeTabId;
    if (const StudioTabEntry *active = findTab(activeId); active && active->isSettings)
        activeId = QString::fromLatin1(kWelcomeTabId);

    QList<StudioTabEntry> persisted;
    persisted.reserve(m_tabs.size());
    for (const StudioTabEntry &tab : m_tabs) {
        if (!tab.isSettings)
            persisted.append(tab);
    }
    m_tabStore.save(persisted, activeId);
}

bool StudioTabController::setActiveTabIdInternal(const QString &id, bool persistNow)
{
    if (!findTab(id))
        return false;
    if (m_activeTabId == id) {
        if (persistNow)
            saveTabsToSettings();
        return true;
    }

    const StudioTabEntry *entry = findTab(id);
    if (entry && (entry->isWelcome || entry->isSettings)) {
        const QString leavingId = m_activeTabId;
        m_activeTabId = id;
        emit activeTabIdChanged();
        emit activeViewModeChanged();
        stashTab(leavingId);
        if (m_converter)
            m_converter->detachActiveTab();
        if (entry->isWelcome)
            scheduleWelcomeRecentRefresh();
        if (persistNow)
            saveTabsToSettings();
        return true;
    }

    stashActiveTab();
    m_activeTabId = id;

    if (entry && !entry->isWelcome && !entry->isSettings && m_converter) {
        m_converter->setActiveTabId(id);
        TabRestoreService::restoreTab(m_converter, *entry);
    }

    emit activeTabIdChanged();
    emit activeViewModeChanged();
    if (persistNow)
        saveTabsToSettings();
    return true;
}

void StudioTabController::initialize(bool restoreLastProject)
{
    const bool wasCleanExit = m_tabStore.wasCleanExit();
    if (!wasCleanExit)
        qCInfo(lcTabs) << "Restoring tabs after unclean exit";
    m_tabStore.markDirtyExit();

    loadTabsFromSettings();
    TabRestoreService::applyStartupTabPolicy(&m_tabs, &m_activeTabId, wasCleanExit);

    const StudioTabEntry *active = findTab(m_activeTabId);
    if (active && !active->isWelcome && !active->isSettings && m_converter) {
        m_converter->setActiveTabId(m_activeTabId);
        TabRestoreService::restoreTab(m_converter, *active);
    } else {
        m_activeTabId = QString::fromLatin1(kWelcomeTabId);
    }

    saveTabsToSettings();
    emit tabsChanged();
    emit activeTabIdChanged();
    emit activeViewModeChanged();

    if (m_converter)
        m_preload.startWarm(&m_tabs, m_converter->previewProvider());

    const bool onlyWelcome = m_tabs.size() <= 1;
    if (!wasCleanExit && onlyWelcome && restoreLastProject && m_converter
        && m_converter->project()->hasRestorableProject()) {
        openProjectTab(QUrl::fromLocalFile(m_converter->project()->lastProjectPath()));
    }
}

void StudioTabController::persist()
{
    stashActiveTab();
    TabPersistence::flushCachesToDisk(m_tabs, m_activeTabId);
    saveTabsToSettings();
    m_tabStore.markCleanExit();
    invalidateWelcomeRecent();
    emit welcomeRecentItemsChanged();
}

QString StudioTabController::activateWelcome()
{
    ensureWelcomeTab();
    setActiveTabIdInternal(QString::fromLatin1(kWelcomeTabId), true);
    return m_activeTabId;
}

QString StudioTabController::openSettingsTab()
{
    for (StudioTabEntry &t : m_tabs) {
        if (t.isSettings) {
            t.title = AppLocale::tr("Settings");
            setActiveTabIdInternal(t.id, true);
            emit tabsChanged();
            return t.id;
        }
    }

    StudioTabEntry tab;
    tab.id = QString::fromLatin1(kSettingsTabId);
    tab.title = AppLocale::tr("Settings");
    tab.isSettings = true;
    m_tabs.append(tab);
    emit tabsChanged();
    setActiveTabIdInternal(tab.id, true);
    return tab.id;
}

QString StudioTabController::openProjectTab(const QUrl &url)
{
    if (!m_converter || url.isEmpty())
        return {};

    const QString path = url.toLocalFile();
    const QString pathKey = tabPathKey(path);
    for (const StudioTabEntry &t : m_tabs) {
        if (t.isWelcome || t.isSettings)
            continue;
        if (!t.projectPath.isEmpty() && tabPathKey(t.projectPath) == pathKey) {
            setActiveTabIdInternal(t.id, true);
            return t.id;
        }
        if (!t.cachePath.isEmpty() && tabPathKey(t.cachePath) == pathKey) {
            setActiveTabIdInternal(t.id, true);
            return t.id;
        }
    }

    const QString previousId = m_activeTabId;
    stashActiveTab();

    StudioTabEntry tab;
    tab.id = createTabId();
    tab.title = QFileInfo(path).completeBaseName();
    tab.projectPath = path;
    m_tabs.append(tab);
    m_activeTabId = tab.id;
    m_converter->setActiveTabId(tab.id);

    if (!m_converter->project()->openProject(url)) {
        m_tabs.removeLast();
        m_activeTabId = previousId;
        TabRestoreService::restoreTab(m_converter, *findTab(previousId));
        emit tabsChanged();
        emit activeTabIdChanged();
        emit activeViewModeChanged();
        return {};
    }

    findTab(tab.id)->title = TabProjectSync::displayTitle(m_converter);
    emit tabsChanged();
    emit activeTabIdChanged();
    emit activeViewModeChanged();
    saveTabsToSettings();
    return tab.id;
}

QString StudioTabController::newProjectTab(const QString &name)
{
    if (!m_converter)
        return {};

    stashActiveTab();

    StudioTabEntry tab;
    tab.id = createTabId();
    tab.title = name.isEmpty() ? AppLocale::tr("Untitled") : name;
    m_tabs.append(tab);
    m_activeTabId = tab.id;

    m_converter->setActiveTabId(tab.id);
    m_converter->clear();
    m_converter->project()->newProject(tab.title);
    findTab(tab.id)->title = tab.title;

    emit tabsChanged();
    emit activeTabIdChanged();
    emit activeViewModeChanged();
    saveTabsToSettings();
    return tab.id;
}

QString StudioTabController::openFileTab(const QString &localPath)
{
    if (localPath.isEmpty())
        return {};
    if (isProjectPath(localPath))
        return openProjectTab(QUrl::fromLocalFile(localPath));

    const bool reuseActiveTab = !activeIsWelcome() && !activeIsSettings() && m_converter
                                && !m_converter->hasImage();
    const QString id = reuseActiveTab ? m_activeTabId : newProjectTab(QString());
    if (id.isEmpty())
        return {};
    if (!m_converter->loadImage(QUrl::fromLocalFile(localPath))) {
        if (!reuseActiveTab)
            closeTab(id);
        return {};
    }
    syncActiveTabTitle();
    return id;
}

bool StudioTabController::activateTab(const QString &id)
{
    if (!findTab(id))
        return false;
    return setActiveTabIdInternal(id, true);
}

bool StudioTabController::closeTab(const QString &id)
{
    StudioTabEntry *tab = findTab(id);
    if (!tab || tab->isWelcome)
        return false;

    const bool wasActive = m_activeTabId == id;
    if (m_converter && m_converter->previewProvider())
        m_converter->previewProvider()->clearTabSlots(id);
    TabPersistence::removeCacheFile(tab->cachePath);
    for (qsizetype i = 0; i < m_tabs.size(); ++i) {
        if (m_tabs.at(i).id == id) {
            m_tabs.removeAt(i);
            break;
        }
    }
    emit tabsChanged();

    if (!wasActive) {
        saveTabsToSettings();
        return true;
    }

    if (m_tabs.isEmpty())
        ensureWelcomeTab();

    QString fallback = QString::fromLatin1(kWelcomeTabId);
    for (const StudioTabEntry &t : m_tabs) {
        if (t.pinned && !t.isWelcome) {
            fallback = t.id;
            break;
        }
    }
    setActiveTabIdInternal(fallback, true);
    return true;
}

void StudioTabController::setTabPinned(const QString &id, bool pinned)
{
    StudioTabEntry *tab = findTab(id);
    if (!tab || tab->pinned == pinned)
        return;
    tab->pinned = pinned;
    emit tabsChanged();
    saveTabsToSettings();
}

void StudioTabController::moveTab(const QString &id, int toIndex)
{
    int from = -1;
    for (int i = 0; i < m_tabs.size(); ++i) {
        if (m_tabs.at(i).id == id) {
            from = i;
            break;
        }
    }
    if (from < 0)
        return;

    if (m_tabs.at(from).isWelcome)
        return;

    const int welcomeOffset = (!m_tabs.isEmpty() && m_tabs.first().isWelcome) ? 1 : 0;
    const int clamped = qBound(welcomeOffset, toIndex, m_tabs.size() - 1);
    if (from == clamped)
        return;

    const StudioTabEntry entry = m_tabs.takeAt(from);
    const int insertAt = qBound(welcomeOffset, clamped, m_tabs.size());
    m_tabs.insert(insertAt, entry);
    emit tabsChanged();
    saveTabsToSettings();
}

void StudioTabController::closeOtherTabs(const QString &id)
{
    StudioTabEntry *keep = findTab(id);
    if (!keep)
        return;

    QList<StudioTabEntry> next;
    next.reserve(m_tabs.size());
    for (const StudioTabEntry &t : m_tabs) {
        if (t.id == id || t.isWelcome)
            next.append(t);
        else {
            if (m_converter && m_converter->previewProvider())
                m_converter->previewProvider()->clearTabSlots(t.id);
            TabPersistence::removeCacheFile(t.cachePath);
        }
    }
    m_tabs = next;
    if (!findTab(m_activeTabId))
        setActiveTabIdInternal(id, true);
    emit tabsChanged();
    saveTabsToSettings();
}

void StudioTabController::syncActiveTabTitle()
{
    StudioTabEntry *active = findTab(m_activeTabId);
    if (!active || active->isWelcome || active->isSettings)
        return;
    active->title = TabProjectSync::displayTitle(m_converter);
    emit tabsChanged();
}

void StudioTabController::relocalizeTabTitles()
{
    ensureWelcomeTab();
    static const QStringList untitledKeys = {
        QStringLiteral("Untitled"),
        QString::fromUtf8("Без названия"),
    };
    for (StudioTabEntry &t : m_tabs) {
        if (t.isWelcome) {
            t.title = AppLocale::tr("Home");
            continue;
        }
        if (t.isSettings) {
            t.title = AppLocale::tr("Settings");
            continue;
        }
        t.title = stripRecoveredPrefix(t.title);
        if (untitledKeys.contains(t.title))
            t.title = AppLocale::tr("Untitled");
    }
    syncActiveTabTitle();
    emit tabsChanged();
}

void StudioTabController::onProjectChanged()
{
    syncActiveTabTitle();
    StudioTabEntry *active = findTab(m_activeTabId);
    if (!active || active->isWelcome || active->isSettings || !m_converter)
        return;
    if (!m_converter->project()->projectFile().isEmpty()) {
        const QString path = m_converter->project()->projectFile().toLocalFile();
        active->projectPath = path;
        if (!active->cachePath.isEmpty()
            && tabPathKey(active->cachePath) != tabPathKey(path)) {
            QFile::remove(active->cachePath);
            active->cachePath.clear();
        }
    }
}
