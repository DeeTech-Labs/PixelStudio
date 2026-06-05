#include "persistence/RecentPreview.h"

#include "persistence/AppPaths.h"

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>

namespace {

QString projectKey(const QString &projectPath)
{
    const QString canonical = QFileInfo(projectPath).canonicalFilePath();
    return canonical.isEmpty() ? QDir::cleanPath(projectPath) : canonical;
}

QString cacheFilePath(const QString &projectPath)
{
    const QByteArray hash = QCryptographicHash::hash(projectKey(projectPath).toUtf8(),
                                                   QCryptographicHash::Sha256);
    const QString name = QString::fromLatin1(hash.toHex().left(16)) + QStringLiteral(".png");
    return AppPaths::recentThumbnailsDir() + QLatin1Char('/') + name;
}

bool isCacheFresh(const QString &projectPath, const QString &cachePath)
{
    const QFileInfo projectInfo(projectPath);
    const QFileInfo cacheInfo(cachePath);
    if (!projectInfo.exists() || !cacheInfo.exists() || cacheInfo.size() <= 0)
        return false;
    return cacheInfo.lastModified() >= projectInfo.lastModified();
}

bool writePngFile(const QString &path, const QByteArray &pngData)
{
    if (pngData.isEmpty())
        return false;
    QDir().mkpath(QFileInfo(path).absolutePath());
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return false;
    file.write(pngData);
    return file.commit();
}

} // namespace

QString RecentPreview::thumbnailPathForProject(const QString &projectPath)
{
    return cacheFilePath(projectPath);
}

QString RecentPreview::resolveThumbnail(const QString &projectPath, const QByteArray &pngData)
{
    if (pngData.isEmpty())
        return {};

    const QString cachePath = cacheFilePath(projectPath);
    if (isCacheFresh(projectPath, cachePath))
        return cachePath;

    QFile existing(cachePath);
    if (existing.open(QIODevice::ReadOnly)) {
        const QByteArray current = existing.readAll();
        if (current == pngData)
            return cachePath;
    }

    if (!writePngFile(cachePath, pngData))
        return {};
    return cachePath;
}
