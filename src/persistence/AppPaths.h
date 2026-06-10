#ifndef PIXELSTUDIO_PERSISTENCE_APPPATHS_H
#define PIXELSTUDIO_PERSISTENCE_APPPATHS_H

#include <QString>

namespace AppPaths {

inline constexpr const char *kOrganization = "DeeTech";
inline constexpr const char *kApplication = "PixelStudio";

// Application data (QStandardPaths::AppDataLocation): settings, session, cache, logs.
QString dataRoot();

// User documents tree: projects, exports, watch folders, referenced assets.
QString userDocumentsRoot();

// Human-readable default paths for UI hints (platform-aware, ~ for home).
QString formatDisplayPath(const QString &absolutePath);
QString defaultDocumentsRootHint();
QString defaultApplicationDataHint();

QString appSettingsFile();
QString sessionSettingsFile();

QString projectsDir();
QString exportsDir();
QString watchDir();

// User-editable translation bundles under dataRoot()/translations.
QString userTranslationsDir();

// Logs and crash reports under dataRoot()/logs.
QString logsDir();

// Derived/runtime data under dataRoot()/cache (regenerable).
QString cacheDir();
QString tabCacheDir();
QString recentThumbnailsDir();
QString runtimePreviewsDir();

bool isTabCachePath(const QString &absolutePath);
bool isTransientCachePath(const QString &absolutePath);
bool isExcludedFromRecentPath(const QString &absolutePath);
bool isInternalDataPath(const QString &absolutePath);

void ensureLayout();
void ensureStorageDirs();

} // namespace AppPaths

#endif // PIXELSTUDIO_PERSISTENCE_APPPATHS_H
