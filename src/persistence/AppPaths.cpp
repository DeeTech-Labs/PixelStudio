#include "persistence/AppPaths.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <QStandardPaths>

namespace AppPaths {

namespace {

QString readIniValue(const QString &key)
{
    const QString ini = dataRoot() + QStringLiteral("/app.ini");
    if (!QFileInfo::exists(ini))
        return QString();
    QSettings settings(ini, QSettings::IniFormat);
    return settings.value(key).toString().trimmed();
}

QString resolveOverrideDir(const QString &iniKey, const QString &defaultDir)
{
    const QString custom = readIniValue(iniKey);
    if (custom.isEmpty())
        return defaultDir;
    QDir dir(custom);
    return dir.absolutePath();
}

bool ensureDir(const QString &path)
{
    if (path.isEmpty())
        return false;
    return QDir().mkpath(path);
}

bool isConfigIni(const QString &fileName)
{
    return fileName.compare(QStringLiteral("app.ini"), Qt::CaseInsensitive) == 0
        || fileName.compare(QStringLiteral("session.ini"), Qt::CaseInsensitive) == 0;
}

int copyMissingFilesRecursive(const QString &fromDir, const QString &toDir)
{
    QDir src(fromDir);
    if (!src.exists())
        return 0;
    ensureDir(toDir);
    int copied = 0;
    const QFileInfoList entries = src.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QFileInfo &fi : entries) {
        if (fi.isFile() && isConfigIni(fi.fileName()))
            continue;
        const QString target = QDir(toDir).absoluteFilePath(fi.fileName());
        if (fi.isDir()) {
            copied += copyMissingFilesRecursive(fi.absoluteFilePath(), target);
            continue;
        }
        if (QFile::exists(target))
            continue;
        if (QFile::copy(fi.absoluteFilePath(), target))
            ++copied;
    }
    return copied;
}

void relocateMisplacedConfigFiles()
{
    const QString canonical = QDir::cleanPath(dataRoot());
    const QStringList names = {QStringLiteral("app.ini"), QStringLiteral("session.ini")};
    const QStringList scanDirs = {
        userDocumentsRoot(),
        projectsDir(),
        exportsDir(),
        watchDir(),
    };
    for (const QString &dir : scanDirs) {
        if (dir.isEmpty() || QDir::cleanPath(dir) == canonical)
            continue;
        for (const QString &name : names) {
            const QString misplaced = QDir(dir).absoluteFilePath(name);
            if (!QFileInfo::exists(misplaced))
                continue;
            const QString target = QDir(canonical).absoluteFilePath(name);
            ensureDir(canonical);
            if (!QFileInfo::exists(target)) {
                QFile::rename(misplaced, target);
            } else {
                QFile::remove(misplaced);
            }
        }
    }
}

void migrateLegacyKey(QSettings &dest, const QSettings &legacy, const QString &key)
{
    if (!dest.contains(key) && legacy.contains(key))
        dest.setValue(key, legacy.value(key));
}

bool migrationCompleted()
{
    return readIniValue(QStringLiteral("app/migrationCompleted")) == QStringLiteral("true");
}

void markMigrationCompleted()
{
    QSettings settings(appSettingsFile(), QSettings::IniFormat);
    settings.setValue(QStringLiteral("app/migrationCompleted"), true);
    settings.sync();
}

void migrateLegacyStore()
{
    const QSettings legacy(QStringLiteral("PixelStudio"), QStringLiteral("PixelStudio"));
    const QStringList keys = legacy.allKeys();
    if (keys.isEmpty())
        return;

    QSettings appStore(appSettingsFile(), QSettings::IniFormat);
    QSettings sessionStore(sessionSettingsFile(), QSettings::IniFormat);

    for (const QString &key : keys) {
        if (key.startsWith(QStringLiteral("app/")))
            migrateLegacyKey(appStore, legacy, key);
        else
            migrateLegacyKey(sessionStore, legacy, key);
    }
    appStore.sync();
    sessionStore.sync();
}

void migrateUserDataFromAppData()
{
    const QString roaming = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    const QStringList legacyRoots = {
        roaming + QStringLiteral("/PixelStudio"),
        roaming + QStringLiteral("/DeTech/PixelStudio"),
        dataRoot(),
    };
    for (const QString &oldRoot : legacyRoots) {
        if (oldRoot.isEmpty() || !QDir(oldRoot).exists())
            continue;
        copyMissingFilesRecursive(oldRoot + QStringLiteral("/projects"), projectsDir());
        copyMissingFilesRecursive(oldRoot + QStringLiteral("/exports"), exportsDir());
        copyMissingFilesRecursive(oldRoot + QStringLiteral("/watch"), watchDir());
    }
}

void migrateLegacyCacheDirs()
{
    const QString root = dataRoot();
    const auto moveDirContents = [&](const QString &fromDir, const QString &toDir) {
        QDir src(fromDir);
        if (!src.exists())
            return;
        ensureDir(toDir);
        const QFileInfoList files = src.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
        for (const QFileInfo &fi : files) {
            const QString target = QDir(toDir).absoluteFilePath(fi.fileName());
            if (QFileInfo::exists(target))
                QFile::remove(fi.absoluteFilePath());
            else
                QFile::rename(fi.absoluteFilePath(), target);
        }
        src.removeRecursively();
    };

    moveDirContents(root + QStringLiteral("/tab_cache"), tabCacheDir());
    moveDirContents(root + QStringLiteral("/recent_previews"), recentThumbnailsDir());
}

void migrateLegacyTranslationsDir()
{
    const QString legacy = dataRoot() + QStringLiteral("/i18n");
    const QString current = userTranslationsDir();
    QDir legacyDir(legacy);
    if (!legacyDir.exists())
        return;

    ensureDir(current);
    const QFileInfoList files = legacyDir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
    for (const QFileInfo &fi : files) {
        const QString target = QDir(current).absoluteFilePath(fi.fileName());
        if (QFileInfo::exists(target))
            continue;
        QFile::rename(fi.absoluteFilePath(), target);
    }

    if (legacyDir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot).isEmpty())
        legacyDir.removeRecursively();
}

} // namespace

QString dataRoot()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
}

QString userDocumentsRoot()
{
    const QString custom = readIniValue(QStringLiteral("app/documentsRoot"));
    if (!custom.isEmpty())
        return QDir(custom).absolutePath();
    const QString docs = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    if (docs.isEmpty())
        return dataRoot() + QStringLiteral("/Documents");
    return docs + QStringLiteral("/PixelStudio");
}

QString appSettingsFile()
{
    return dataRoot() + QStringLiteral("/app.ini");
}

QString sessionSettingsFile()
{
    return dataRoot() + QStringLiteral("/session.ini");
}

QString projectsDir()
{
    const QString base = userDocumentsRoot();
    return resolveOverrideDir(QStringLiteral("app/projectsRoot"), base + QStringLiteral("/projects"));
}

QString exportsDir()
{
    const QString base = userDocumentsRoot();
    return resolveOverrideDir(QStringLiteral("app/exportsRoot"), base + QStringLiteral("/exports"));
}

QString watchDir()
{
    return userDocumentsRoot() + QStringLiteral("/watch");
}

QString userTranslationsDir()
{
    return dataRoot() + QStringLiteral("/translations");
}

QString cacheDir()
{
    return dataRoot() + QStringLiteral("/cache");
}

QString tabCacheDir()
{
    return cacheDir() + QStringLiteral("/tabs");
}

QString recentThumbnailsDir()
{
    return cacheDir() + QStringLiteral("/recent");
}

QString runtimePreviewsDir()
{
    return cacheDir() + QStringLiteral("/runtime");
}

namespace {

QString cacheRelativePath(const QString &absolutePath)
{
    if (absolutePath.isEmpty())
        return QString();
    const QString native = QDir::cleanPath(QDir::fromNativeSeparators(absolutePath));
    const QString data = QDir::cleanPath(dataRoot());
    if (!native.startsWith(data, Qt::CaseInsensitive))
        return QString();
    QString rel = native.mid(data.size()).trimmed();
    if (rel.startsWith(QLatin1Char('/')) || rel.startsWith(QLatin1Char('\\')))
        rel = rel.mid(1);
    return rel;
}

} // namespace

bool isTabCachePath(const QString &absolutePath)
{
    const QString rel = cacheRelativePath(absolutePath);
    return rel.startsWith(QStringLiteral("cache/tabs/"), Qt::CaseInsensitive);
}

bool isTransientCachePath(const QString &absolutePath)
{
    const QString rel = cacheRelativePath(absolutePath);
    return rel.startsWith(QStringLiteral("cache/runtime/"), Qt::CaseInsensitive)
        || rel.startsWith(QStringLiteral("cache/recent/"), Qt::CaseInsensitive);
}

bool isExcludedFromRecentPath(const QString &absolutePath)
{
    return isTransientCachePath(absolutePath);
}

bool isInternalDataPath(const QString &absolutePath)
{
    return isTabCachePath(absolutePath) || isTransientCachePath(absolutePath);
}

void ensureStorageDirs()
{
    ensureDir(userDocumentsRoot());
    ensureDir(projectsDir());
    ensureDir(exportsDir());
    ensureDir(watchDir());
}

void ensureLayout()
{
    ensureDir(dataRoot());
    ensureDir(cacheDir());
    ensureDir(tabCacheDir());
    ensureDir(recentThumbnailsDir());
    ensureDir(runtimePreviewsDir());
    ensureStorageDirs();
    migrateLegacyTranslationsDir();
    ensureDir(userTranslationsDir());

    if (!migrationCompleted()) {
        migrateLegacyStore();
        migrateUserDataFromAppData();
        migrateLegacyCacheDirs();
        markMigrationCompleted();
    } else {
        migrateLegacyCacheDirs();
    }
    relocateMisplacedConfigFiles();
}

} // namespace AppPaths
