#include "persistence/StoredPath.h"

#include "persistence/AppPaths.h"

#include <QDir>
#include <QFileInfo>

namespace StoredPath {

namespace {

bool isAbsoluteOnDisk(const QString &path)
{
    if (path.isEmpty())
        return false;
    const QString native = QDir::fromNativeSeparators(path);
    return QFileInfo(native).isAbsolute();
}

} // namespace

QString encode(const QString &absolutePath)
{
    if (absolutePath.isEmpty())
        return QString();
    const QString native = QDir::cleanPath(QDir::fromNativeSeparators(absolutePath));
    if (!isAbsoluteOnDisk(native))
        return native;

    const QString root = QDir::cleanPath(AppPaths::userDocumentsRoot());
    if (!native.startsWith(root, Qt::CaseInsensitive))
        return native;

    QString rel = native.mid(root.size());
    if (rel.startsWith(QLatin1Char('/')) || rel.startsWith(QLatin1Char('\\')))
        rel = rel.mid(1);
    return rel.isEmpty() ? QStringLiteral(".") : rel;
}

QString decode(const QString &storedPath)
{
    if (storedPath.isEmpty())
        return QString();
    const QString native = QDir::fromNativeSeparators(storedPath);
    if (isAbsoluteOnDisk(native))
        return QDir::cleanPath(native);

    const QString root = QDir::cleanPath(AppPaths::userDocumentsRoot());
    return QDir::cleanPath(root + QLatin1Char('/') + native);
}

QStringList encodeList(const QStringList &absolutePaths)
{
    QStringList out;
    out.reserve(absolutePaths.size());
    for (const QString &path : absolutePaths)
        out.append(encode(path));
    return out;
}

QStringList decodeList(const QStringList &storedPaths)
{
    QStringList out;
    out.reserve(storedPaths.size());
    for (const QString &path : storedPaths)
        out.append(decode(path));
    return out;
}

} // namespace StoredPath
