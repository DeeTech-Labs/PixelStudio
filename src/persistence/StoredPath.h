#ifndef PIXELSTUDIO_PERSISTENCE_STOREDPATH_H
#define PIXELSTUDIO_PERSISTENCE_STOREDPATH_H

#include <QString>

namespace StoredPath {

QString encode(const QString &absolutePath);
QString decode(const QString &storedPath);

QStringList encodeList(const QStringList &absolutePaths);
QStringList decodeList(const QStringList &storedPaths);

} // namespace StoredPath

#endif // PIXELSTUDIO_PERSISTENCE_STOREDPATH_H
