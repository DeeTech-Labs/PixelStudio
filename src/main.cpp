#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QLocale>
#include <QQuickStyle>
#include <QColor>
#include <QUrl>
#include <QQuickWindow>
#include <QStyleHints>
#include <QTimer>
#include <QDir>
#include <QFileInfo>

#ifdef Q_OS_WIN
#include <windows.h>
#include <dwmapi.h>
#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif
#endif
#include "app/AppTranslations.h"
#include "app/AppVersion.h"
#include "app/DisplayConverter.h"
#include "app/StudioTabController.h"
#include "app/WinTaskbarRecent.h"
#include "persistence/AppPaths.h"
#include "persistence/AppSettings.h"
#include "persistence/ProjectFormat.h"
#include "persistence/SessionSettings.h"

int main(int argc, char *argv[])
{
    QQuickStyle::setStyle("Fusion");
#ifdef Q_OS_WIN
    WinTaskbarRecent::installIdentity();
#endif
    QGuiApplication app(argc, argv);
    if (QStyleHints *hints = app.styleHints())
        hints->setColorScheme(Qt::ColorScheme::Dark);
    app.setOrganizationName(QString::fromLatin1(AppPaths::kOrganization));
    app.setOrganizationDomain(QStringLiteral("deetech.local"));
    app.setApplicationName(QString::fromLatin1(AppPaths::kApplication));
    const QString appVersion = AppVersion::display();
    app.setApplicationVersion(appVersion);
    AppPaths::ensureLayout();

    AppSettings appSettings;
    AppTranslations::install(app, appSettings.languageCode());
    SessionSettings session;
    DisplayConverter converter(&session, &appSettings);
    StudioTabController tabController(&converter);
    WinTaskbarRecent winTaskbarRecent;

    QQmlApplicationEngine engine;
    engine.setUiLanguage(AppTranslations::effectiveLanguageCode(appSettings.languageCode()));
    engine.rootContext()->setContextProperty("applicationVersion", appVersion);
    engine.rootContext()->setContextProperty("converter", &converter);
    engine.rootContext()->setContextProperty("tabController", &tabController);
    engine.rootContext()->setContextProperty("appSettings", &appSettings);
    engine.rootContext()->setContextProperty("pixelStudioDataPath",
                                              QUrl::fromLocalFile(AppPaths::dataRoot()));
    engine.rootContext()->setContextProperty("pixelStudioDocumentsPath",
                                              QUrl::fromLocalFile(AppPaths::userDocumentsRoot()));
    engine.rootContext()->setContextProperty("pixelStudioProjectsUrl",
                                              QUrl::fromLocalFile(AppPaths::projectsDir()));
    engine.rootContext()->setContextProperty("pixelStudioExportsUrl",
                                              QUrl::fromLocalFile(AppPaths::exportsDir()));
    engine.rootContext()->setContextProperty("pixelStudioWatchUrl",
                                              QUrl::fromLocalFile(AppPaths::watchDir()));

    const auto syncTaskbarRecent = [&]() {
        winTaskbarRecent.syncFromRecentFiles(converter.recentFiles());
    };
    QObject::connect(&converter, &DisplayConverter::recentFilesChanged, &app, syncTaskbarRecent);
    syncTaskbarRecent();

    QObject::connect(&appSettings, &AppSettings::languageCodeChanged, &engine, [&]() {
        const QString code = AppTranslations::effectiveLanguageCode(appSettings.languageCode());
        AppTranslations::setLanguage(app, appSettings.languageCode());
        engine.setUiLanguage(code);
        converter.refreshLocalization();
        tabController.relocalizeTabTitles();
        engine.retranslate();
    });

    QObject::connect(&app, &QGuiApplication::aboutToQuit, &tabController, [&tabController, &converter]() {
        tabController.persist();
        converter.flushPersistence();
    });

    engine.loadFromModule("PixelStudio", "Main");
    engine.retranslate();

    tabController.initialize(appSettings.restoreLastProject() && !appSettings.showWelcomeOnStartup());

    const auto openLocalPath = [&](const QString &localPath) {
        if (localPath.isEmpty() || !QFileInfo::exists(localPath))
            return;
        if (ProjectFormat::isProjectPath(localPath))
            tabController.openProjectTab(QUrl::fromLocalFile(localPath));
        else
            tabController.openFileTab(localPath);
    };
    const QStringList args = app.arguments();
    for (int i = 1; i < args.size(); ++i) {
        const QString &arg = args.at(i);
        if (arg.startsWith(QLatin1Char('-')))
            continue;
        openLocalPath(QDir::toNativeSeparators(arg));
    }

    const QList<QObject *> roots = engine.rootObjects();
    for (QObject *obj : roots) {
        if (auto *window = qobject_cast<QQuickWindow *>(obj)) {
            window->setColor(QColor(0x1A, 0x1A, 0x1A));
            QTimer::singleShot(0, window, [window, syncTaskbarRecent]() {
#ifdef Q_OS_WIN
                const HWND hwnd = reinterpret_cast<HWND>(window->winId());
                if (hwnd) {
                    const BOOL dark = TRUE;
                    DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &dark, sizeof(dark));
                }
#endif
                syncTaskbarRecent();
            });
            break;
        }
    }

    return app.exec();
}
