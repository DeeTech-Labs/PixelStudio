#ifndef PIXELSTUDIO_PERSISTENCE_RECENTPREVIEW_H
#define PIXELSTUDIO_PERSISTENCE_RECENTPREVIEW_H

#include <QByteArray>
#include <QImage>
#include <QString>

namespace RecentPreview {

QString thumbnailPathForProject(const QString &projectPath);
QString resolveThumbnail(const QString &projectPath, const QByteArray &pngData);

} // namespace RecentPreview

#endif // PIXELSTUDIO_PERSISTENCE_RECENTPREVIEW_H
