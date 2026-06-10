#include "persistence/StoredPath.h"

#include "persistence/AppPaths.h"
#include "persistence/PathCompare.h"

#include <QDir>
#include <QFileInfo>

namespace StoredPath {

namespace {

constexpr QLatin1StringView kDataPrefix("@data/");

bool isAbsoluteOnDisk(const QString &path)
{
    if (path.isEmpty())
        return false;
    const QString native = QDir::fromNativeSeparators(path);
    return QFileInfo(native).isAbsolute();
}

QString relativeToRoot(const QString &root, const QString &native)
{
    QString rel = native.mid(root.size());
    if (rel.startsWith(QLatin1Char('/')) || rel.startsWith(QLatin1Char('\\')))
        rel = rel.mid(1);
    return rel.isEmpty() ? QStringLiteral(".") : rel;
}

} // namespace

QString encode(const QString &absolutePath)
{
    if (absolutePath.isEmpty())
        return QString();
    const QString native = QDir::cleanPath(QDir::fromNativeSeparators(absolutePath));
    if (!isAbsoluteOnDisk(native))
        return native;

    const QString docsRoot = QDir::cleanPath(AppPaths::userDocumentsRoot());
    if (PathCompare::startsWithRoot(native, docsRoot))
        return relativeToRoot(docsRoot, native);

    const QString dataRoot = QDir::cleanPath(AppPaths::dataRoot());
    if (PathCompare::startsWithRoot(native, dataRoot))
        return QString(kDataPrefix) + relativeToRoot(dataRoot, native);

    return native;
}

QString decode(const QString &storedPath)
{
    if (storedPath.isEmpty())
        return QString();
    if (storedPath.startsWith(kDataPrefix))
        return QDir::cleanPath(AppPaths::dataRoot() + QLatin1Char('/')
                               + storedPath.mid(kDataPrefix.size()));

    const QString native = QDir::fromNativeSeparators(storedPath);
    if (isAbsoluteOnDisk(native))
        return QDir::cleanPath(native);

    const QString docsRoot = QDir::cleanPath(AppPaths::userDocumentsRoot());
    return QDir::cleanPath(docsRoot + QLatin1Char('/') + native);
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
