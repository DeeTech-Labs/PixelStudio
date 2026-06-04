#include "persistence/AppPaths.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <QStandardPaths>

namespace AppPaths {

namespace {

QString g_testDataRoot;
QString g_testDocumentsRoot;

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
            if (!QFile::exists(misplaced))
                continue;
            const QString target = QDir(canonical).absoluteFilePath(name);
            ensureDir(canonical);
            if (!QFile::exists(target)) {
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

} // namespace

void setTestRoots(const QString &dataRoot, const QString &documentsRoot)
{
    g_testDataRoot = dataRoot;
    g_testDocumentsRoot = documentsRoot;
}

void clearTestRoots()
{
    g_testDataRoot.clear();
    g_testDocumentsRoot.clear();
}

QString dataRoot()
{
    if (!g_testDataRoot.isEmpty())
        return g_testDataRoot;
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
}

QString userDocumentsRoot()
{
    if (!g_testDocumentsRoot.isEmpty())
        return g_testDocumentsRoot;
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

QString tabCacheDir()
{
    return dataRoot() + QStringLiteral("/tab_cache");
}

void ensureLayout()
{
    ensureDir(dataRoot());
    ensureDir(tabCacheDir());
    ensureDir(userDocumentsRoot());
    ensureDir(projectsDir());
    ensureDir(exportsDir());
    ensureDir(watchDir());

    if (!migrationCompleted()) {
        migrateLegacyStore();
        migrateUserDataFromAppData();
        markMigrationCompleted();
    }
    relocateMisplacedConfigFiles();
}

} // namespace AppPaths
