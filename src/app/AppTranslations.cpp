#include "app/AppTranslations.h"

#include <QCoreApplication>
#include <QDir>
#include <QGuiApplication>
#include <QLibraryInfo>
#include <QLocale>
#include <QTranslator>

namespace {

QTranslator s_qtTranslator;
QTranslator s_appTranslator;

QString systemLocaleCode()
{
    const QStringList langs = QLocale::system().uiLanguages();
    if (!langs.isEmpty())
        return langs.first().section(QLatin1Char('-'), 0, 0).toLower();
    return QLocale::system().bcp47Name().section(QLatin1Char('-'), 0, 0).toLower();
}

QString normalizeLanguageCode(const QString &languageCode)
{
    if (languageCode == QStringLiteral("ru") || languageCode == QStringLiteral("en"))
        return languageCode;
    const QString system = systemLocaleCode();
    return system == QStringLiteral("ru") ? QStringLiteral("ru") : QStringLiteral("en");
}

bool tryLoadAppQm(QTranslator &translator, const QString &code)
{
    if (translator.load(QLocale(code), QStringLiteral("pixelstudio"),
                        QStringLiteral("_"), QStringLiteral(":/i18n"))) {
        return true;
    }
    const QString file = QStringLiteral("pixelstudio_%1.qm").arg(code);
    const QStringList paths = {
        QStringLiteral(":/i18n/") + file,
        QStringLiteral(":/") + file,
        QCoreApplication::applicationDirPath() + QStringLiteral("/i18n/") + file,
        QCoreApplication::applicationDirPath() + QLatin1Char('/') + file,
    };
    for (const QString &path : paths) {
        if (translator.load(path))
            return true;
    }
    return false;
}

} // namespace

namespace AppTranslations {

void install(QGuiApplication &app, const QString &languageCode)
{
    setLanguage(app, languageCode);
}

void setLanguage(QGuiApplication &app, const QString &languageCode)
{
    app.removeTranslator(&s_qtTranslator);
    app.removeTranslator(&s_appTranslator);

    const QString effectiveCode = normalizeLanguageCode(languageCode);
    const QLocale locale(effectiveCode);
    if (s_qtTranslator.load(locale,
                          QStringLiteral("qt"),
                          QStringLiteral("_"),
                          QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
        app.installTranslator(&s_qtTranslator);
    }

    if (!tryLoadAppQm(s_appTranslator, effectiveCode))
        tryLoadAppQm(s_appTranslator, QStringLiteral("en"));

    app.installTranslator(&s_appTranslator);
    QLocale::setDefault(locale);
}

QString activeLanguageCode()
{
    return QLocale().name().section(QLatin1Char('_'), 0, 0).toLower();
}

QString effectiveLanguageCode(const QString &languageCode)
{
    return normalizeLanguageCode(languageCode);
}

} // namespace AppTranslations
