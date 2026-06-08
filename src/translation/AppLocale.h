#ifndef PIXELSTUDIO_TRANSLATION_APPLOCALE_H
#define PIXELSTUDIO_TRANSLATION_APPLOCALE_H

#include <QCoreApplication>
#include <QString>

namespace AppLocale {

inline constexpr const char *kContext = "Core";

inline QString tr(const char *text)
{
    return QCoreApplication::translate(kContext, text);
}

} // namespace AppLocale

#endif // PIXELSTUDIO_TRANSLATION_APPLOCALE_H
