#ifndef PIXELSTUDIO_IO_IMAGEFORMATS_H
#define PIXELSTUDIO_IO_IMAGEFORMATS_H

#include <QString>
#include <QStringList>

namespace ImageFormats {

QStringList supportedExtensions();
QStringList watchGlobPatterns();
bool isSupportedPath(const QString &path);
bool isSvgPath(const QString &path);
QString extensionPattern();
QString openDialogFilter();

} // namespace ImageFormats

#endif // PIXELSTUDIO_IO_IMAGEFORMATS_H
