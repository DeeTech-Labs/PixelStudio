#include "persistence/AppSettings.h"

#include "app/code/CodeSyntaxTheme.h"
#include "translation/AppLocale.h"
#include "translation/TranslationStore.h"
#include "persistence/AppPaths.h"
#include "persistence/SettingsSchema.h"

#include <QVariantMap>

AppSettings::AppSettings(QObject *parent)
    : QObject(parent)
    , m_settings(AppPaths::appSettingsFile(), QSettings::IniFormat)
    , m_codeSyntax(new CodeSyntaxTheme(this))
{
    SettingsSchema::migrateAppSettings(m_settings);
    load();
}

void AppSettings::load()
{
    m_languageCode = m_settings.value(QStringLiteral("app/language"), m_languageCode).toString();
    if (m_languageCode != QStringLiteral("system")
        && !TranslationStore::instance().hasLanguage(m_languageCode)) {
        m_languageCode = QStringLiteral("system");
    }
    m_showSidebar = m_settings.value(QStringLiteral("app/showSidebar"), m_showSidebar).toBool();
    m_showPixelGrid = m_settings.value(QStringLiteral("app/showPixelGrid"), m_showPixelGrid).toBool();
    m_codeWrap = m_settings.value(QStringLiteral("app/codeWrap"), m_codeWrap).toBool();
    m_confirmExit = m_settings.value(QStringLiteral("app/confirmExit"), m_confirmExit).toBool();
    m_confirmCloseTab = m_settings.value(QStringLiteral("app/confirmCloseTab"), m_confirmCloseTab).toBool();
    m_restoreLastProject = m_settings.value(QStringLiteral("app/restoreLastProject"), m_restoreLastProject).toBool();
    m_showWelcomeOnStartup = m_settings.value(QStringLiteral("app/showWelcomeOnStartup"), m_showWelcomeOnStartup).toBool();
    m_projectAutosave = m_settings.value(QStringLiteral("app/projectAutosave"), m_projectAutosave).toBool();
    m_projectAutosaveSeconds = qBound(30, m_settings.value(QStringLiteral("app/projectAutosaveSeconds"), m_projectAutosaveSeconds).toInt(), 3600);
    m_projectsRoot = m_settings.value(QStringLiteral("app/projectsRoot")).toString();
    m_exportsRoot = m_settings.value(QStringLiteral("app/exportsRoot")).toString();
    m_documentsRoot = m_settings.value(QStringLiteral("app/documentsRoot")).toString();
    m_codeSyntax->load(m_settings);
}

void AppSettings::saveValue(const QString &key, const QVariant &value)
{
    m_settings.setValue(key, value);
    m_settings.sync();
}

QVariantList AppSettings::availableLanguages() const
{
    return TranslationStore::instance().availableLanguages();
}

void AppSettings::resetUiDefaults()
{
    setShowSidebar(true);
    setShowPixelGrid(true);
    setCodeWrap(false);
    setConfirmExit(true);
    setConfirmCloseTab(true);
    m_codeSyntax->resetDefaults();
}

void AppSettings::setLanguageCode(const QString &code)
{
    QString safe = code;
    if (safe != QStringLiteral("system")) {
        safe = safe.toLower();
        if (!TranslationStore::instance().hasLanguage(safe))
            safe = QStringLiteral("system");
    }
    if (m_languageCode == safe)
        return;
    m_languageCode = safe;
    saveValue(QStringLiteral("app/language"), m_languageCode);
    emit languageCodeChanged();
}

void AppSettings::setShowSidebar(bool on)
{
    if (m_showSidebar == on)
        return;
    m_showSidebar = on;
    saveValue(QStringLiteral("app/showSidebar"), on);
    emit showSidebarChanged();
}

void AppSettings::setShowPixelGrid(bool on)
{
    if (m_showPixelGrid == on)
        return;
    m_showPixelGrid = on;
    saveValue(QStringLiteral("app/showPixelGrid"), on);
    emit showPixelGridChanged();
}

void AppSettings::setCodeWrap(bool on)
{
    if (m_codeWrap == on)
        return;
    m_codeWrap = on;
    saveValue(QStringLiteral("app/codeWrap"), on);
    emit codeWrapChanged();
}

void AppSettings::setConfirmExit(bool on)
{
    if (m_confirmExit == on)
        return;
    m_confirmExit = on;
    saveValue(QStringLiteral("app/confirmExit"), on);
    emit confirmExitChanged();
}

void AppSettings::setConfirmCloseTab(bool on)
{
    if (m_confirmCloseTab == on)
        return;
    m_confirmCloseTab = on;
    saveValue(QStringLiteral("app/confirmCloseTab"), on);
    emit confirmCloseTabChanged();
}

void AppSettings::setRestoreLastProject(bool on)
{
    if (m_restoreLastProject == on)
        return;
    m_restoreLastProject = on;
    saveValue(QStringLiteral("app/restoreLastProject"), on);
    emit restoreLastProjectChanged();
}

void AppSettings::setShowWelcomeOnStartup(bool on)
{
    if (m_showWelcomeOnStartup == on)
        return;
    m_showWelcomeOnStartup = on;
    saveValue(QStringLiteral("app/showWelcomeOnStartup"), on);
    emit showWelcomeOnStartupChanged();
}

void AppSettings::setProjectAutosave(bool on)
{
    if (m_projectAutosave == on)
        return;
    m_projectAutosave = on;
    saveValue(QStringLiteral("app/projectAutosave"), on);
    emit projectAutosaveChanged();
}

void AppSettings::setProjectAutosaveSeconds(int seconds)
{
    const int safe = qBound(30, seconds, 3600);
    if (m_projectAutosaveSeconds == safe)
        return;
    m_projectAutosaveSeconds = safe;
    saveValue(QStringLiteral("app/projectAutosaveSeconds"), safe);
    emit projectAutosaveSecondsChanged();
}

void AppSettings::setProjectsRoot(const QString &path)
{
    if (m_projectsRoot == path)
        return;
    m_projectsRoot = path;
    saveValue(QStringLiteral("app/projectsRoot"), path);
    emit projectsRootChanged();
}

void AppSettings::setExportsRoot(const QString &path)
{
    if (m_exportsRoot == path)
        return;
    m_exportsRoot = path;
    saveValue(QStringLiteral("app/exportsRoot"), path);
    emit exportsRootChanged();
}

void AppSettings::setDocumentsRoot(const QString &path)
{
    if (m_documentsRoot == path)
        return;
    m_documentsRoot = path;
    saveValue(QStringLiteral("app/documentsRoot"), path);
    emit documentsRootChanged();
}

void AppSettings::reloadFromDisk()
{
    m_settings.sync();
    load();
    emit languageCodeChanged();
    emit showSidebarChanged();
    emit showPixelGridChanged();
    emit codeWrapChanged();
    emit confirmExitChanged();
    emit confirmCloseTabChanged();
    emit restoreLastProjectChanged();
    emit showWelcomeOnStartupChanged();
    emit projectAutosaveChanged();
    emit projectAutosaveSecondsChanged();
    emit projectsRootChanged();
    emit exportsRootChanged();
    emit documentsRootChanged();
}
