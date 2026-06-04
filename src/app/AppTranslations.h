#ifndef APPTRANSLATIONS_H
#define APPTRANSLATIONS_H

#include <QString>

class QGuiApplication;

namespace AppTranslations {
void install(QGuiApplication &app, const QString &languageCode = QStringLiteral("system"));
void setLanguage(QGuiApplication &app, const QString &languageCode);
QString activeLanguageCode();
QString effectiveLanguageCode(const QString &languageCode);
}

#endif // APPTRANSLATIONS_H
