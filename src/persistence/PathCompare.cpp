#include "persistence/PathCompare.h"

#include <QDir>

namespace PathCompare {

bool startsWithRoot(const QString &absolutePath, const QString &rootDir)
{
    if (absolutePath.isEmpty() || rootDir.isEmpty())
        return false;

    const QString path = QDir::cleanPath(QDir::fromNativeSeparators(absolutePath));
    const QString root = QDir::cleanPath(QDir::fromNativeSeparators(rootDir));
    if (path.size() < root.size())
        return false;
    if (!path.startsWith(root, prefixSensitivity()))
        return false;
    if (path.size() == root.size())
        return true;
    const QChar next = path.at(root.size());
    return next == QLatin1Char('/') || next == QLatin1Char('\\');
}

} // namespace PathCompare
