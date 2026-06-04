#ifndef PIXELSTUDIO_PERSISTENCE_APPPATHS_H
#define PIXELSTUDIO_PERSISTENCE_APPPATHS_H

#include <QString>

namespace AppPaths {

inline constexpr const char *kOrganization = "DeeTech";
inline constexpr const char *kApplication = "PixelStudio";

void setTestRoots(const QString &dataRoot, const QString &documentsRoot);
void clearTestRoots();

QString dataRoot();
QString userDocumentsRoot();

QString appSettingsFile();
QString sessionSettingsFile();

QString projectsDir();
QString exportsDir();
QString watchDir();
QString tabCacheDir();

void ensureLayout();

} // namespace AppPaths

#endif // PIXELSTUDIO_PERSISTENCE_APPPATHS_H
