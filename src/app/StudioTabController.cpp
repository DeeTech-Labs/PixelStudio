#include "app/StudioTabController.h"

#include "app/DisplayConverter.h"
#include "i18n/AppLocale.h"
#include "persistence/AppPaths.h"
#include "persistence/ProjectFormat.h"
#include "persistence/ProjectService.h"

#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QUuid>

namespace {

constexpr auto kTabsKey = "app/tabs";
constexpr auto kActiveTabKey = "app/activeTabId";
constexpr auto kCleanExitKey = "app/cleanExit";

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
    , m_settings(AppPaths::appSettingsFile(), QSettings::IniFormat)
{
    setConverter(converter);
}

void StudioTabController::setConverter(DisplayConverter *converter)
{
    if (m_converter) {
        disconnect(m_converter, nullptr, this, nullptr);
    }
    m_converter = converter;
    if (m_converter) {
        connect(m_converter, &DisplayConverter::projectChanged, this, &StudioTabController::onProjectChanged);
    }
}

QVariantList StudioTabController::tabsToVariant() const
{
    QVariantList out;
    out.reserve(m_tabs.size());
    for (const TabEntry &t : m_tabs) {
        out.append(QVariantMap{
            {QStringLiteral("id"), t.id},
            {QStringLiteral("title"), formatTabTitle(t)},
            {QStringLiteral("pinned"), t.pinned},
            {QStringLiteral("isWelcome"), t.isWelcome},
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

bool StudioTabController::activeIsWelcome() const
{
    const TabEntry *t = findTab(m_activeTabId);
    return !t || t->isWelcome;
}

QString StudioTabController::activeTabTitle() const
{
    const TabEntry *t = findTab(m_activeTabId);
    return t ? t->title : QString();
}

QString StudioTabController::normalizePathKey(const QString &path)
{
    return tabPathKey(path);
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

QString StudioTabController::formatTabTitle(const TabEntry &entry) const
{
    QString title = entry.title;
    if (title.isEmpty())
        title = entry.isWelcome ? AppLocale::tr("Home") : AppLocale::tr("Untitled");
    if (!entry.recovered || entry.isWelcome)
        return title;
    const QString prefix = AppLocale::tr("[Recovered]");
    if (title.startsWith(prefix))
        return title;
    return prefix + QLatin1Char(' ') + title;
}

int StudioTabController::activeViewMode() const
{
    const TabEntry *t = findTab(m_activeTabId);
    return t ? normalizeViewMode(t->viewMode) : 0;
}

void StudioTabController::setActiveViewMode(int mode)
{
    TabEntry *t = findTab(m_activeTabId);
    if (!t)
        return;
    const int clamped = normalizeViewMode(mode);
    if (t->viewMode == clamped)
        return;
    t->viewMode = clamped;
    emit activeViewModeChanged();
    if (!t->isWelcome)
        saveTabsToSettings();
}

StudioTabController::TabEntry *StudioTabController::findTab(const QString &id)
{
    for (TabEntry &t : m_tabs) {
        if (t.id == id)
            return &t;
    }
    return nullptr;
}

const StudioTabController::TabEntry *StudioTabController::findTab(const QString &id) const
{
    for (const TabEntry &t : m_tabs) {
        if (t.id == id)
            return &t;
    }
    return nullptr;
}

QString StudioTabController::createTabId()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

QString StudioTabController::displayTitle() const
{
    if (!m_converter)
        return QString();
    if (!m_converter->projectFile().isEmpty())
        return QFileInfo(m_converter->projectFile().toLocalFile()).completeBaseName();
    if (!m_converter->projectName().isEmpty() && m_converter->projectName() != QStringLiteral("Untitled"))
        return m_converter->projectName();
    if (m_converter->hasImage() && !m_converter->sourcePath().isEmpty()) {
        const QString local = m_converter->sourcePath().toLocalFile();
        if (!local.isEmpty())
            return QFileInfo(local).fileName();
    }
    return m_converter->projectName().isEmpty() ? AppLocale::tr("Untitled") : m_converter->projectName();
}

StudioTabController::TabEntry &StudioTabController::ensureWelcomeTab()
{
    for (TabEntry &t : m_tabs) {
        if (t.isWelcome) {
            t.title = AppLocale::tr("Home");
            return t;
        }
    }
    TabEntry welcome;
    welcome.id = QString::fromLatin1(kWelcomeTabId);
    welcome.title = AppLocale::tr("Home");
    welcome.isWelcome = true;
    welcome.pinned = true;
    m_tabs.prepend(welcome);
    return m_tabs.first();
}

void StudioTabController::stashActiveTab()
{
    if (!m_converter)
        return;
    TabEntry *active = findTab(m_activeTabId);
    if (!active || active->isWelcome)
        return;

    m_converter->flushPersistence();
    active->title = displayTitle();

    if (!m_converter->projectFile().isEmpty()) {
        active->projectPath = m_converter->projectFile().toLocalFile();
        active->cachePath.clear();
        m_converter->saveProject();
        return;
    }

    if (!m_converter->hasImage() && m_converter->projectName().isEmpty())
        return;

    if (active->cachePath.isEmpty()) {
        active->cachePath = AppPaths::tabCacheDir() + QLatin1Char('/')
            + active->id + ProjectFormat::extension();
    }
    m_converter->saveProjectAs(QUrl::fromLocalFile(active->cachePath));
    active->projectPath.clear();
}

bool StudioTabController::restoreTab(const TabEntry &entry)
{
    if (!m_converter || entry.isWelcome)
        return true;

    if (!entry.projectPath.isEmpty() && QFileInfo::exists(entry.projectPath))
        return m_converter->openProject(QUrl::fromLocalFile(entry.projectPath));

    if (!entry.cachePath.isEmpty() && QFileInfo::exists(entry.cachePath))
        return m_converter->openProject(QUrl::fromLocalFile(entry.cachePath));

    m_converter->clear();
    m_converter->newProject(entry.title.isEmpty() ? AppLocale::tr("Untitled") : entry.title);
    return true;
}

void StudioTabController::removeTabCache(const TabEntry &entry)
{
    if (!entry.cachePath.isEmpty())
        QFile::remove(entry.cachePath);
}

void StudioTabController::applyStartupTabPolicy(bool wasCleanExit)
{
    ensureWelcomeTab();

    QString nextActive = m_activeTabId;
    if (wasCleanExit) {
        const TabEntry *active = findTab(nextActive);
        if (active && !active->isWelcome && !active->pinned)
            nextActive = QString::fromLatin1(kWelcomeTabId);
    }

    QList<TabEntry> kept;
    kept.reserve(m_tabs.size());
    for (const TabEntry &t : m_tabs) {
        if (t.isWelcome) {
            kept.append(t);
            continue;
        }
        if (wasCleanExit) {
            if (t.pinned)
                kept.append(t);
            else
                removeTabCache(t);
        } else {
            TabEntry copy = t;
            if (!copy.pinned)
                copy.recovered = true;
            kept.append(copy);
        }
    }
    m_tabs = kept;

    if (!findTab(nextActive)) {
        nextActive = QString::fromLatin1(kWelcomeTabId);
        for (const TabEntry &t : m_tabs) {
            if (t.pinned && !t.isWelcome) {
                nextActive = t.id;
                break;
            }
        }
    }
    m_activeTabId = nextActive;
}

void StudioTabController::loadTabsFromSettings()
{
    m_tabs.clear();
    const QVariantList stored = m_settings.value(QLatin1String(kTabsKey)).toList();
    for (const QVariant &v : stored) {
        const QVariantMap m = v.toMap();
        TabEntry t;
        t.id = m.value(QStringLiteral("id")).toString();
        if (t.id.isEmpty())
            continue;
        t.title = m.value(QStringLiteral("title")).toString();
        t.pinned = m.value(QStringLiteral("pinned")).toBool();
        t.isWelcome = m.value(QStringLiteral("isWelcome")).toBool();
        t.projectPath = m.value(QStringLiteral("projectPath")).toString();
        t.cachePath = m.value(QStringLiteral("cachePath")).toString();
        t.viewMode = normalizeViewMode(m.value(QStringLiteral("viewMode"), 0).toInt());
        if (t.isWelcome)
            t.id = QString::fromLatin1(kWelcomeTabId);
        t.title = sanitizeTabTitle(t.title);
        if (t.title.isEmpty())
            t.title = t.isWelcome ? AppLocale::tr("Home") : AppLocale::tr("Untitled");
        m_tabs.append(t);
    }
    ensureWelcomeTab();
    m_activeTabId = m_settings.value(QLatin1String(kActiveTabKey), QString::fromLatin1(kWelcomeTabId)).toString();
    if (!findTab(m_activeTabId))
        m_activeTabId = QString::fromLatin1(kWelcomeTabId);
}

void StudioTabController::saveTabsToSettings()
{
    QVariantList stored;
    stored.reserve(m_tabs.size());
    for (const TabEntry &t : m_tabs) {
        stored.append(QVariantMap{
            {QStringLiteral("id"), t.id},
            {QStringLiteral("title"), t.title},
            {QStringLiteral("pinned"), t.pinned},
            {QStringLiteral("isWelcome"), t.isWelcome},
            {QStringLiteral("projectPath"), t.projectPath},
            {QStringLiteral("cachePath"), t.cachePath},
            {QStringLiteral("viewMode"), t.viewMode},
        });
    }
    m_settings.setValue(QLatin1String(kTabsKey), stored);
    m_settings.setValue(QLatin1String(kActiveTabKey), m_activeTabId);
    m_settings.sync();
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

    stashActiveTab();
    m_activeTabId = id;

    const TabEntry *entry = findTab(id);
    if (entry && entry->isWelcome && m_converter)
        m_converter->clear();
    else if (entry && !entry->isWelcome)
        restoreTab(*entry);

    emit activeTabIdChanged();
    emit activeViewModeChanged();
    if (persistNow)
        saveTabsToSettings();
    return true;
}

void StudioTabController::initialize(bool restoreLastProject)
{
    const bool wasCleanExit = m_settings.value(QLatin1String(kCleanExitKey), true).toBool();
    m_settings.setValue(QLatin1String(kCleanExitKey), false);
    m_settings.sync();

    loadTabsFromSettings();
    applyStartupTabPolicy(wasCleanExit);
    saveTabsToSettings();
    emit tabsChanged();

    const TabEntry *active = findTab(m_activeTabId);
    if (active && !active->isWelcome)
        restoreTab(*active);
    else
        m_activeTabId = QString::fromLatin1(kWelcomeTabId);

    emit activeTabIdChanged();
    emit activeViewModeChanged();

    const bool onlyWelcome = m_tabs.size() <= 1;
    if (!wasCleanExit && onlyWelcome && restoreLastProject && m_converter
        && m_converter->hasRestorableProject()) {
        openProjectTab(QUrl::fromLocalFile(m_converter->lastProjectPath()));
    }
}

void StudioTabController::persist()
{
    stashActiveTab();
    saveTabsToSettings();
    m_settings.setValue(QLatin1String(kCleanExitKey), true);
    m_settings.sync();
}

QString StudioTabController::activateWelcome()
{
    ensureWelcomeTab();
    setActiveTabIdInternal(QString::fromLatin1(kWelcomeTabId), true);
    return m_activeTabId;
}

QString StudioTabController::openProjectTab(const QUrl &url)
{
    if (!m_converter || url.isEmpty())
        return {};

    const QString path = url.toLocalFile();
    const QString pathKey = tabPathKey(path);
    for (const TabEntry &t : m_tabs) {
        if (t.isWelcome)
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

    TabEntry tab;
    tab.id = createTabId();
    tab.title = QFileInfo(path).completeBaseName();
    tab.projectPath = path;
    m_tabs.append(tab);
    m_activeTabId = tab.id;

    if (!m_converter->openProject(url)) {
        m_tabs.removeLast();
        m_activeTabId = previousId;
        restoreTab(*findTab(previousId));
        emit tabsChanged();
        emit activeTabIdChanged();
        emit activeViewModeChanged();
        return {};
    }

    findTab(tab.id)->title = displayTitle();
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

    TabEntry tab;
    tab.id = createTabId();
    tab.title = name.isEmpty() ? AppLocale::tr("Untitled") : name;
    m_tabs.append(tab);
    m_activeTabId = tab.id;

    m_converter->clear();
    m_converter->newProject(tab.title);
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

    const QString id = newProjectTab(QString());
    if (id.isEmpty())
        return {};
    if (!m_converter->loadImage(QUrl::fromLocalFile(localPath))) {
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
    TabEntry *tab = findTab(id);
    if (!tab || tab->isWelcome)
        return false;

    const bool wasActive = m_activeTabId == id;
    removeTabCache(*tab);
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
    for (const TabEntry &t : m_tabs) {
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
    TabEntry *tab = findTab(id);
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

    const TabEntry entry = m_tabs.takeAt(from);
    const int insertAt = qBound(welcomeOffset, clamped, m_tabs.size());
    m_tabs.insert(insertAt, entry);
    emit tabsChanged();
    saveTabsToSettings();
}

void StudioTabController::closeOtherTabs(const QString &id)
{
    TabEntry *keep = findTab(id);
    if (!keep)
        return;

    QList<TabEntry> next;
    next.reserve(m_tabs.size());
    for (const TabEntry &t : m_tabs) {
        if (t.id == id || t.isWelcome)
            next.append(t);
        else
            removeTabCache(t);
    }
    m_tabs = next;
    if (!findTab(m_activeTabId))
        setActiveTabIdInternal(id, true);
    emit tabsChanged();
    saveTabsToSettings();
}

void StudioTabController::syncActiveTabTitle()
{
    TabEntry *active = findTab(m_activeTabId);
    if (!active || active->isWelcome)
        return;
    active->title = displayTitle();
    emit tabsChanged();
}

void StudioTabController::relocalizeTabTitles()
{
    ensureWelcomeTab();
    static const QStringList untitledKeys = {
        QStringLiteral("Untitled"),
        QString::fromUtf8("Без названия"),
    };
    for (TabEntry &t : m_tabs) {
        if (t.isWelcome)
            continue;
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
    TabEntry *active = findTab(m_activeTabId);
    if (!active || active->isWelcome || !m_converter)
        return;
    if (!m_converter->projectFile().isEmpty()) {
        active->projectPath = m_converter->projectFile().toLocalFile();
        if (!active->cachePath.isEmpty()) {
            QFile::remove(active->cachePath);
            active->cachePath.clear();
        }
    }
}
