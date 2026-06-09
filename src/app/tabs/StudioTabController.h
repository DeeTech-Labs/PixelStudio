#ifndef PIXELSTUDIO_APP_STUDIOTABCONTROLLER_H
#define PIXELSTUDIO_APP_STUDIOTABCONTROLLER_H

#include <QObject>
#include <QUrl>
#include <QVariantList>

#include "app/tabs/TabSessionStore.h"
#include "app/tabs/TabTypes.h"

class DisplayConverter;

class StudioTabController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList tabs READ tabs NOTIFY tabsChanged)
    Q_PROPERTY(QString activeTabId READ activeTabId NOTIFY activeTabIdChanged)
    Q_PROPERTY(QString activeTabTitle READ activeTabTitle NOTIFY tabsChanged)
    Q_PROPERTY(bool activeIsWelcome READ activeIsWelcome NOTIFY activeTabIdChanged)
    Q_PROPERTY(int activeViewMode READ activeViewMode WRITE setActiveViewMode NOTIFY activeViewModeChanged)
    Q_PROPERTY(QVariantList welcomeRecentItems READ welcomeRecentItems NOTIFY welcomeRecentItemsChanged)

public:
    static constexpr const char *kWelcomeTabId = "welcome";

    explicit StudioTabController(DisplayConverter *converter, QObject *parent = nullptr);

    QVariantList tabs() const;
    QString activeTabId() const { return m_activeTabId; }
    QString activeTabTitle() const;
    bool activeIsWelcome() const;
    int activeViewMode() const;
    void setActiveViewMode(int mode);
    QVariantList welcomeRecentItems() const;

    void setConverter(DisplayConverter *converter);

    Q_INVOKABLE void initialize(bool restoreLastProject = false);
    Q_INVOKABLE void persist();
    Q_INVOKABLE QString activateWelcome();
    Q_INVOKABLE QString openProjectTab(const QUrl &url);
    Q_INVOKABLE QString newProjectTab(const QString &name = QString());
    Q_INVOKABLE QString openFileTab(const QString &localPath);
    Q_INVOKABLE bool activateTab(const QString &id);
    Q_INVOKABLE bool closeTab(const QString &id);
    Q_INVOKABLE void setTabPinned(const QString &id, bool pinned);
    Q_INVOKABLE void moveTab(const QString &id, int toIndex);
    Q_INVOKABLE void closeOtherTabs(const QString &id);
    Q_INVOKABLE void syncActiveTabTitle();
    Q_INVOKABLE void relocalizeTabTitles();

public slots:
    void onProjectChanged();

signals:
    void tabsChanged();
    void activeTabIdChanged();
    void activeViewModeChanged();
    void welcomeRecentItemsChanged();
    void tabActionFailed(const QString &message);

private:
    StudioTabEntry *findTab(const QString &id);
    const StudioTabEntry *findTab(const QString &id) const;
    StudioTabEntry &ensureWelcomeTab();
    QString createTabId();
    static bool looksLikeUuidTitle(const QString &title);
    static QString sanitizeTabTitle(const QString &title);
    void stashActiveTab();
    void stashTab(const QString &tabId);
    void invalidateWelcomeRecent();
    void scheduleWelcomeRecentRefresh();
    void loadTabsFromSettings();
    void saveTabsToSettings();
    QString formatTabTitle(const StudioTabEntry &entry) const;
    static QString stripRecoveredPrefix(const QString &title);
    QVariantList tabsToVariant() const;
    bool setActiveTabIdInternal(const QString &id, bool persistNow);

    DisplayConverter *m_converter = nullptr;
    TabSessionStore m_tabStore;
    QList<StudioTabEntry> m_tabs;
    QString m_activeTabId;
    mutable QVariantList m_welcomeRecentCache;
    mutable bool m_welcomeRecentDirty = true;
};

#endif // PIXELSTUDIO_APP_STUDIOTABCONTROLLER_H
