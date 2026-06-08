#include "app/AppTranslations.h"

#include "translation/JsonTranslator.h"
#include "translation/TranslationStore.h"

#include <QCoreApplication>
#include <QGuiApplication>
#include <QLibraryInfo>
#include <QLocale>
#include <QTranslator>

namespace {

QTranslator s_qtTranslator;
JsonTranslator s_appTranslator(&TranslationStore::instance());

QString systemLocaleCode()
{
    const QStringList langs = QLocale::system().uiLanguages();
    if (!langs.isEmpty())
        return langs.first().section(QLatin1Char('-'), 0, 0).toLower();
    return QLocale::system().bcp47Name().section(QLatin1Char('-'), 0, 0).toLower();
}

} // namespace

namespace AppTranslations {

void install(QGuiApplication &app, const QString &languageCode)
{
    TranslationStore::instance().refreshCatalog();
    setLanguage(app, languageCode);
}

void setLanguage(QGuiApplication &app, const QString &languageCode)
{
    app.removeTranslator(&s_qtTranslator);
    app.removeTranslator(&s_appTranslator);

    TranslationStore::instance().loadLanguage(languageCode);

    const QString effectiveCode = TranslationStore::instance().effectiveLanguageCode(languageCode);
    const QLocale locale(effectiveCode);
    if (s_qtTranslator.load(locale,
                          QStringLiteral("qt"),
                          QStringLiteral("_"),
                          QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
        app.installTranslator(&s_qtTranslator);
    }

    app.installTranslator(&s_appTranslator);
    QLocale::setDefault(locale);
}

QString activeLanguageCode()
{
    const QString loaded = TranslationStore::instance().activeLanguageCode();
    if (!loaded.isEmpty())
        return loaded;
    return QLocale().name().section(QLatin1Char('_'), 0, 0).toLower();
}

QString effectiveLanguageCode(const QString &languageCode)
{
    return TranslationStore::instance().effectiveLanguageCode(languageCode);
}

} // namespace AppTranslations
