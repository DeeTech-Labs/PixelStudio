#include "io/ImageFormats.h"

#include <QFileInfo>

namespace ImageFormats {

namespace {

constexpr const char *kExtensions[] = {
    "png",
    "jpg",
    "jpeg",
    "bmp",
    "gif",
    "webp",
    "svg",
    "svgz",
    "tif",
    "tiff",
    "ico",
    "pbm",
    "pgm",
    "ppm",
    "xbm",
    "xpm",
};

} // namespace

QStringList supportedExtensions()
{
    QStringList out;
    out.reserve(int(sizeof(kExtensions) / sizeof(kExtensions[0])));
    for (const char *ext : kExtensions)
        out.append(QString::fromLatin1(ext));
    return out;
}

QStringList watchGlobPatterns()
{
    QStringList patterns;
    patterns.reserve(supportedExtensions().size());
    for (const QString &ext : supportedExtensions())
        patterns.append(QStringLiteral("*.%1").arg(ext));
    return patterns;
}

bool isSupportedPath(const QString &path)
{
    const QString ext = QFileInfo(path).suffix().toLower();
    if (ext.isEmpty())
        return false;
    for (const char *supported : kExtensions) {
        if (ext == QLatin1String(supported))
            return true;
    }
    return false;
}

bool isSvgPath(const QString &path)
{
    const QString ext = QFileInfo(path).suffix().toLower();
    return ext == QLatin1String("svg") || ext == QLatin1String("svgz");
}

QString extensionPattern()
{
    return QStringLiteral("(*.png *.jpg *.jpeg *.bmp *.gif *.webp *.svg *.svgz *.tif *.tiff *.ico *.pbm *.pgm *.ppm *.xbm *.xpm)");
}

QString openDialogFilter()
{
    return QStringLiteral("Images ") + extensionPattern();
}

} // namespace ImageFormats
