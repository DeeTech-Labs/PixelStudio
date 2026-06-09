#include "app/tabs/TabWelcomeService.h"

#include "app/studio/DisplayConverter.h"
#include "app/studio/controllers/project/ProjectController.h"
#include "persistence/ProjectFormat.h"
#include "persistence/ProjectService.h"
#include "persistence/SessionSettings.h"

#include <QDateTime>
#include <QFileInfo>
#include <algorithm>

namespace {

QString tabPathKey(const QString &path)
{
    if (path.isEmpty())
        return {};
    const QString canonical = QFileInfo(path).canonicalFilePath();
    return canonical.isEmpty() ? path : canonical;
}

} // namespace

QVariantMap TabWelcomeService::recentRowForTab(const StudioTabEntry &tab)
{
    auto finish = [&](QVariantMap row) -> QVariantMap {
        if (row.isEmpty())
            return row;
        row.insert(QStringLiteral("tabId"), tab.id);
        return row;
    };

    if (tab.tabSnapshot.has_value()) {
        if (tab.tabSnapshot->sourceImage.isNull()
            && !ProjectService::hasImageContent(tab.tabSnapshot->project)) {
            return {};
        }

        const QString imagePath = !tab.tabSnapshot->sourceFilePath.isEmpty()
            ? tab.tabSnapshot->sourceFilePath
            : ProjectService::primaryImagePath(tab.tabSnapshot->project);
        if (!imagePath.isEmpty())
            return finish(SessionSettings::makeRecentEntry(imagePath, tab.title));

        QString path;
        if (!tab.projectPath.isEmpty() && QFileInfo::exists(tab.projectPath))
            path = tab.projectPath;
        else if (!tab.cachePath.isEmpty() && QFileInfo::exists(tab.cachePath))
            path = tab.cachePath;
        if (!path.isEmpty())
            return finish(SessionSettings::makeRecentEntry(path, tab.title, true));
        return {};
    }

    QString path;
    if (!tab.projectPath.isEmpty() && QFileInfo::exists(tab.projectPath))
        path = tab.projectPath;
    else if (!tab.cachePath.isEmpty() && QFileInfo::exists(tab.cachePath))
        path = tab.cachePath;
    else
        return {};
    return finish(SessionSettings::makeRecentEntry(path, tab.title, true));
}

QVariantList TabWelcomeService::mergedRecentItems(DisplayConverter *converter,
                                                  const QList<StudioTabEntry> &tabs)
{
    QHash<QString, QVariantMap> rows;
    QHash<QString, QDateTime> modified;

    const auto ingest = [&](const QVariantMap &row, const QDateTime &mtimeHint = {}) {
        const QString path = row.value(QStringLiteral("path")).toString();
        const QString tabId = row.value(QStringLiteral("tabId")).toString();
        if (path.isEmpty() && tabId.isEmpty())
            return;
        const QString key = !path.isEmpty() ? tabPathKey(path) : QStringLiteral("tab:") + tabId;
        const QDateTime mtime = mtimeHint.isValid()
            ? mtimeHint
            : (path.isEmpty() ? QDateTime::currentDateTime() : QFileInfo(path).lastModified());
        if (!rows.contains(key) || modified.value(key) < mtime) {
            rows.insert(key, row);
            modified.insert(key, mtime);
        }
    };

    if (converter) {
        for (const QVariant &item : converter->project()->recentFiles()) {
            const QVariantMap row = item.toMap();
            const QString path = row.value(QStringLiteral("path")).toString();
            if (path.isEmpty())
                continue;
            if (ProjectFormat::isProjectPath(path)) {
                const QVariantMap filtered = SessionSettings::makeRecentEntry(path, QString(), true);
                if (filtered.isEmpty())
                    continue;
                ingest(filtered);
            } else {
                ingest(row);
            }
        }
    }

    for (const StudioTabEntry &tab : tabs) {
        if (tab.isWelcome)
            continue;
        const QVariantMap row = recentRowForTab(tab);
        if (!row.isEmpty())
            ingest(row);
    }

    QStringList keys = rows.keys();
    std::sort(keys.begin(), keys.end(), [&](const QString &a, const QString &b) {
        return modified.value(a) > modified.value(b);
    });

    QVariantList out;
    out.reserve(keys.size());
    for (const QString &key : keys)
        out.append(rows.value(key));
    return out;
}
