#ifndef PIXELSTUDIO_PERSISTENCE_APPSETTINGS_H
#define PIXELSTUDIO_PERSISTENCE_APPSETTINGS_H

#include <QObject>
#include <QSettings>
#include <QVariantList>

class CodeSyntaxTheme;

class AppSettings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString languageCode READ languageCode WRITE setLanguageCode NOTIFY languageCodeChanged)
    Q_PROPERTY(bool showSidebar READ showSidebar WRITE setShowSidebar NOTIFY showSidebarChanged)
    Q_PROPERTY(bool showPixelGrid READ showPixelGrid WRITE setShowPixelGrid NOTIFY showPixelGridChanged)
    Q_PROPERTY(bool codeWrap READ codeWrap WRITE setCodeWrap NOTIFY codeWrapChanged)
    Q_PROPERTY(bool confirmExit READ confirmExit WRITE setConfirmExit NOTIFY confirmExitChanged)
    Q_PROPERTY(bool confirmCloseTab READ confirmCloseTab WRITE setConfirmCloseTab NOTIFY confirmCloseTabChanged)
    Q_PROPERTY(bool restoreLastProject READ restoreLastProject WRITE setRestoreLastProject NOTIFY restoreLastProjectChanged)
    Q_PROPERTY(bool showWelcomeOnStartup READ showWelcomeOnStartup WRITE setShowWelcomeOnStartup NOTIFY showWelcomeOnStartupChanged)
    Q_PROPERTY(bool projectAutosave READ projectAutosave WRITE setProjectAutosave NOTIFY projectAutosaveChanged)
    Q_PROPERTY(int projectAutosaveSeconds READ projectAutosaveSeconds WRITE setProjectAutosaveSeconds NOTIFY projectAutosaveSecondsChanged)
    Q_PROPERTY(QString projectsRoot READ projectsRoot WRITE setProjectsRoot NOTIFY projectsRootChanged)
    Q_PROPERTY(QString exportsRoot READ exportsRoot WRITE setExportsRoot NOTIFY exportsRootChanged)
    Q_PROPERTY(QString documentsRoot READ documentsRoot WRITE setDocumentsRoot NOTIFY documentsRootChanged)
    Q_PROPERTY(CodeSyntaxTheme *codeSyntax READ codeSyntax CONSTANT)

public:
    explicit AppSettings(QObject *parent = nullptr);

    CodeSyntaxTheme *codeSyntax() const { return m_codeSyntax; }

    QString languageCode() const { return m_languageCode; }
    bool showSidebar() const { return m_showSidebar; }
    bool showPixelGrid() const { return m_showPixelGrid; }
    bool codeWrap() const { return m_codeWrap; }
    bool confirmExit() const { return m_confirmExit; }
    bool confirmCloseTab() const { return m_confirmCloseTab; }
    bool restoreLastProject() const { return m_restoreLastProject; }
    bool showWelcomeOnStartup() const { return m_showWelcomeOnStartup; }
    bool projectAutosave() const { return m_projectAutosave; }
    int projectAutosaveSeconds() const { return m_projectAutosaveSeconds; }
    QString projectsRoot() const { return m_projectsRoot; }
    QString exportsRoot() const { return m_exportsRoot; }
    QString documentsRoot() const { return m_documentsRoot; }

    Q_INVOKABLE QVariantList availableLanguages() const;
    Q_INVOKABLE void resetUiDefaults();
    void reloadFromDisk();

public slots:
    void setLanguageCode(const QString &code);
    void setShowSidebar(bool on);
    void setShowPixelGrid(bool on);
    void setCodeWrap(bool on);
    void setConfirmExit(bool on);
    void setConfirmCloseTab(bool on);
    void setRestoreLastProject(bool on);
    void setShowWelcomeOnStartup(bool on);
    void setProjectAutosave(bool on);
    void setProjectAutosaveSeconds(int seconds);
    void setProjectsRoot(const QString &path);
    void setExportsRoot(const QString &path);
    void setDocumentsRoot(const QString &path);

signals:
    void languageCodeChanged();
    void showSidebarChanged();
    void showPixelGridChanged();
    void codeWrapChanged();
    void confirmExitChanged();
    void confirmCloseTabChanged();
    void restoreLastProjectChanged();
    void showWelcomeOnStartupChanged();
    void projectAutosaveChanged();
    void projectAutosaveSecondsChanged();
    void projectsRootChanged();
    void exportsRootChanged();
    void documentsRootChanged();

private:
    void load();
    void saveValue(const QString &key, const QVariant &value);

    QSettings m_settings;
    QString m_languageCode = QStringLiteral("system");
    bool m_showSidebar = true;
    bool m_showPixelGrid = true;
    bool m_codeWrap = false;
    bool m_confirmExit = true;
    bool m_confirmCloseTab = true;
    bool m_restoreLastProject = true;
    bool m_showWelcomeOnStartup = true;
    bool m_projectAutosave = true;
    int m_projectAutosaveSeconds = 120;
    QString m_projectsRoot;
    QString m_exportsRoot;
    QString m_documentsRoot;
    CodeSyntaxTheme *m_codeSyntax = nullptr;
};

#endif // PIXELSTUDIO_PERSISTENCE_APPSETTINGS_H
