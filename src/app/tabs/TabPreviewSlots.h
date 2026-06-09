#ifndef PIXELSTUDIO_APP_TABS_TABPREVIEWSLOTS_H
#define PIXELSTUDIO_APP_TABS_TABPREVIEWSLOTS_H

#include <QString>

namespace TabPreviewSlots {

inline QString source(const QString &tabId)
{
    return QStringLiteral("tab/") + tabId + QStringLiteral("/source");
}

inline QString preview(const QString &tabId)
{
    return QStringLiteral("tab/") + tabId + QStringLiteral("/preview");
}

inline QString process(const QString &tabId)
{
    return QStringLiteral("tab/") + tabId + QStringLiteral("/process");
}

} // namespace TabPreviewSlots

#endif // PIXELSTUDIO_APP_TABS_TABPREVIEWSLOTS_H
