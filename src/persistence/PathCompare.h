#ifndef PIXELSTUDIO_PERSISTENCE_PATHCOMPARE_H
#define PIXELSTUDIO_PERSISTENCE_PATHCOMPARE_H

#include <QtGlobal>

#include <QString>

namespace PathCompare {

inline Qt::CaseSensitivity prefixSensitivity()
{
#if defined(Q_OS_WIN)
    return Qt::CaseInsensitive;
#else
    return Qt::CaseSensitive;
#endif
}

bool startsWithRoot(const QString &absolutePath, const QString &rootDir);

} // namespace PathCompare

#endif // PIXELSTUDIO_PERSISTENCE_PATHCOMPARE_H
