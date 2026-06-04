#ifndef PIXELSTUDIO_I18N_APPLOCALE_H
#define PIXELSTUDIO_I18N_APPLOCALE_H

#include <QCoreApplication>
#include <QString>

namespace AppLocale {

inline constexpr const char *kContext = "PixelStudio";

inline QString tr(const char *text)
{
    return QCoreApplication::translate(kContext, text);
}

} // namespace AppLocale

#endif // PIXELSTUDIO_I18N_APPLOCALE_H
