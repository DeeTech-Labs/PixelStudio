#ifndef PIXELSTUDIO_PERSISTENCE_APPPATHS_H
#define PIXELSTUDIO_PERSISTENCE_APPPATHS_H

#include <QString>

namespace AppPaths {

inline constexpr const char *kOrganization = "DeeTech";
inline constexpr const char *kApplication = "PixelStudio";

// %AppData%/DeeTech/PixelStudio — settings, session, derived cache (no user projects).
QString dataRoot();

// Documents/PixelStudio — projects, exports, watch folders, referenced assets.
QString userDocumentsRoot();

QString appSettingsFile();
QString sessionSettingsFile();

QString projectsDir();
QString exportsDir();
QString watchDir();

// User-editable translation bundles (%AppData%/DeeTech/PixelStudio/translations).
QString userTranslationsDir();

// Application logs and crash dumps under dataRoot()/logs.
QString logsDir();

// Derived/runtime data under dataRoot()/cache (regenerable, not authoritative).
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
