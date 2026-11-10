#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QtQml/qqml.h>
#include <QtGlobal>
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
#include "app/shell/AppTranslations.h"
#include "app/shell/StudioViewMode.h"
#include "app/shell/WorkspaceContext.h"
#include "app/panels/InspectorDisplayPresenter.h"
#include "app/panels/InspectorImagePresenter.h"
#include "app/AppVersion.h"
#include "app/code/CodeSyntaxHighlighter.h"
#include "app/studio/DisplayConverter.h"
#include "app/studio/controllers/project/ProjectController.h"
#include "app/preview/PreviewImageProvider.h"
#include "app/tabs/StudioTabController.h"
#include "app/shell/WinTaskbarRecent.h"
#include "translation/TranslationStore.h"
#include "persistence/AppPaths.h"
#include "persistence/AppSettings.h"
#include "persistence/ProjectFormat.h"
#include "persistence/SessionSettings.h"
#include "io/ImageFormatsProvider.h"
#include "logging/AppLogger.h"
#include "logging/CrashHandler.h"

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
    AppLogger::install();
    CrashHandler::install();
    AppLogger::logSessionStart(appVersion);
    TranslationStore::instance().refreshCatalog();

    AppSettings appSettings;
    AppTranslations::install(app, appSettings.languageCode());
    SessionSettings session;
    auto *previewProvider = new PreviewImageProvider;
    DisplayConverter converter(&session, &appSettings);
    StudioTabController tabController(&converter);
    WorkspaceContext workspace(&converter, &tabController, &appSettings);
    InspectorImagePresenter inspectorImage(&converter);
    InspectorDisplayPresenter inspectorDisplay(&converter);
    WinTaskbarRecent winTaskbarRecent;
    ImageFormatsProvider imageFormats;

    qmlRegisterUncreatableMetaObject(StudioViewMode::staticMetaObject,
                                     "PixelStudio",
                                     1,
                                     0,
                                     "StudioViewMode",
                                     QStringLiteral("enum"));

    QQmlApplicationEngine engine;
    engine.addImageProvider(QLatin1String(PreviewImageProvider::kProviderId), previewProvider);
    converter.setPreviewProvider(previewProvider);
    qmlRegisterType<CodeSyntaxHighlighter>("PixelStudio", 1, 0, "CodeSyntaxHighlighter");
    engine.setUiLanguage(AppTranslations::effectiveLanguageCode(appSettings.languageCode()));
    engine.rootContext()->setContextProperty("applicationVersion", appVersion);
    engine.rootContext()->setContextProperty("qtRuntimeVersion",
                                              QString::fromLatin1(qVersion()));
    engine.rootContext()->setContextProperty("workspace", &workspace);
    engine.rootContext()->setContextProperty("inspectorImage", &inspectorImage);
    engine.rootContext()->setContextProperty("inspectorDisplay", &inspectorDisplay);
    engine.rootContext()->setContextProperty("pixelStudioDataPath",
                                              QUrl::fromLocalFile(AppPaths::dataRoot()));
    engine.rootContext()->setContextProperty("imageFormats", &imageFormats);

    const auto syncTaskbarRecent = [&]() {
        winTaskbarRecent.syncFromRecentFiles(converter.project()->recentFiles());
    };
    QObject::connect(converter.project(), &ProjectController::recentFilesChanged, &app, syncTaskbarRecent);
    syncTaskbarRecent();

    QObject::connect(&appSettings, &AppSettings::languageCodeChanged, &engine, [&]() {
        const QString code = AppTranslations::effectiveLanguageCode(appSettings.languageCode());
        AppTranslations::setLanguage(app, appSettings.languageCode());
        engine.setUiLanguage(code);
        converter.refreshLocalization();
        tabController.relocalizeTabTitles();
        engine.retranslate();
    });

    QObject::connect(&appSettings, &AppSettings::storagePathsChanged, &converter, [&converter]() {
        converter.exportPanel()->applyStoredWatchState();
        converter.project()->notifyUiFoldersChanged();
        converter.exportPanel()->notifyUiFoldersChanged();
    });

    QObject::connect(&app, &QGuiApplication::aboutToQuit, &tabController, [&tabController, &converter]() {
        AppLogger::logSessionEnd();
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
            window->setColor(QColor(0x0d, 0x11, 0x17));
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
