#include "logging/AppLogger.h"

#include "persistence/AppPaths.h"

#include "logging/CrashHandler.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#include <QCoreApplication>
#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QLoggingCategory>
#include <QMutex>
#include <QMutexLocker>
#include <QProcessEnvironment>
#include <QSysInfo>
#include <QTextStream>

#include <cstdio>
#include <cstdlib>

#ifdef Q_OS_WIN
#include <io.h>
#else
#include <fcntl.h>
#include <unistd.h>
#endif

namespace {

constexpr qint64 kMaxLogFileBytes = 5 * 1024 * 1024;
constexpr int kRotatedLogCount = 4;

QMutex g_logMutex;
AppLogger::Level g_minLevel = AppLogger::Level::Info;
bool g_installed = false;
bool g_consoleLogging = false;

QString logBaseName()
{
    return QStringLiteral("PixelStudio.log");
}

QString rotatedLogName(int index)
{
    return QStringLiteral("PixelStudio.%1.log").arg(index);
}

QString levelLabel(QtMsgType type)
{
    switch (type) {
    case QtDebugMsg:
        return QStringLiteral("DEBUG");
    case QtInfoMsg:
        return QStringLiteral("INFO");
    case QtWarningMsg:
        return QStringLiteral("WARN");
    case QtCriticalMsg:
        return QStringLiteral("ERROR");
    case QtFatalMsg:
        return QStringLiteral("FATAL");
    }
    return QStringLiteral("UNKNOWN");
}

AppLogger::Level defaultMinLevel()
{
#if defined(QT_DEBUG) || !defined(NDEBUG)
    return AppLogger::Level::Debug;
#else
    return AppLogger::Level::Info;
#endif
}

bool consoleLoggingEnabled()
{
    return g_consoleLogging;
}

void initConsoleLogging()
{
    const QString env = QProcessEnvironment::systemEnvironment()
                            .value(QStringLiteral("PIXELSTUDIO_LOG_CONSOLE"));
    if (env == QStringLiteral("1")
        || env.compare(QStringLiteral("true"), Qt::CaseInsensitive) == 0) {
        g_consoleLogging = true;
#ifdef Q_OS_WIN
        if (AttachConsole(ATTACH_PARENT_PROCESS)) {
            freopen("CONOUT$", "w", stdout);
            freopen("CONOUT$", "w", stderr);
        }
#endif
        return;
    }
#if defined(QT_DEBUG) || !defined(NDEBUG)
    g_consoleLogging = true;
#endif
}

AppLogger::Level parseLevelToken(const QString &token)
{
    const QString normalized = token.trimmed().toLower();
    if (normalized == QStringLiteral("debug"))
        return AppLogger::Level::Debug;
    if (normalized == QStringLiteral("info"))
        return AppLogger::Level::Info;
    if (normalized == QStringLiteral("warn") || normalized == QStringLiteral("warning"))
        return AppLogger::Level::Warning;
    if (normalized == QStringLiteral("error") || normalized == QStringLiteral("critical"))
        return AppLogger::Level::Error;
    if (normalized == QStringLiteral("fatal"))
        return AppLogger::Level::Fatal;
    return defaultMinLevel();
}

bool levelAllows(AppLogger::Level minLevel, QtMsgType type)
{
    switch (minLevel) {
    case AppLogger::Level::Debug:
        return true;
    case AppLogger::Level::Info:
        return type != QtDebugMsg;
    case AppLogger::Level::Warning:
        return type == QtWarningMsg || type == QtCriticalMsg || type == QtFatalMsg;
    case AppLogger::Level::Error:
        return type == QtCriticalMsg || type == QtFatalMsg;
    case AppLogger::Level::Fatal:
        return type == QtFatalMsg;
    }
    return true;
}

void rotateLogsIfNeeded(const QString &logPath)
{
    QFileInfo info(logPath);
    if (!info.exists() || info.size() < kMaxLogFileBytes)
        return;

    for (int index = kRotatedLogCount; index >= 1; --index) {
        const QString from = index == 1
            ? logPath
            : AppPaths::logsDir() + QLatin1Char('/') + rotatedLogName(index - 1);
        const QString to = AppPaths::logsDir() + QLatin1Char('/') + rotatedLogName(index);
        if (QFile::exists(to))
            QFile::remove(to);
        if (QFile::exists(from))
            QFile::rename(from, to);
    }
}

void appendToLogFileDirect(const QString &line)
{
    const QString logPath = AppLogger::logFilePath();
    const QByteArray payload = line.toUtf8() + '\n';

#ifdef Q_OS_WIN
    HANDLE file = CreateFileW(reinterpret_cast<LPCWSTR>(logPath.utf16()),
                              FILE_APPEND_DATA,
                              FILE_SHARE_READ | FILE_SHARE_WRITE,
                              nullptr,
                              OPEN_ALWAYS,
                              FILE_ATTRIBUTE_NORMAL,
                              nullptr);
    if (file == INVALID_HANDLE_VALUE)
        return;

    DWORD written = 0;
    WriteFile(file, payload.constData(), static_cast<DWORD>(payload.size()), &written, nullptr);
    FlushFileBuffers(file);
    CloseHandle(file);
#else
    const int fd = ::open(logPath.toUtf8().constData(),
                          O_WRONLY | O_CREAT | O_APPEND,
                          0644);
    if (fd < 0)
        return;
    const ssize_t written = ::write(fd, payload.constData(), static_cast<size_t>(payload.size()));
    if (written > 0)
        ::fsync(fd);
    ::close(fd);
#endif
}

void appendToLogFile(const QString &line)
{
    const QString logPath = AppLogger::logFilePath();
    rotateLogsIfNeeded(logPath);

    QFile file(logPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
        return;

    QTextStream stream(&file);
    stream << line << QLatin1Char('\n');
    stream.flush();
}

bool stdioStreamUsable(FILE *stream)
{
    if (!stream)
        return false;
#ifdef Q_OS_WIN
    const int fd = _fileno(stream);
    if (fd < 0)
        return false;
    const HANDLE handle = reinterpret_cast<HANDLE>(_get_osfhandle(fd));
    if (handle == INVALID_HANDLE_VALUE || handle == nullptr)
        return false;
    const DWORD fileType = GetFileType(handle);
    return fileType == FILE_TYPE_CHAR || fileType == FILE_TYPE_PIPE;
#else
    return isatty(fileno(stream)) != 0;
#endif
}

void writeToConsole(QtMsgType type, const QString &line)
{
    if (!consoleLoggingEnabled())
        return;

    FILE *out = (type == QtCriticalMsg || type == QtFatalMsg) ? stderr : stdout;
    const QByteArray payload = line.toLocal8Bit() + '\n';

    if (stdioStreamUsable(out)) {
        fwrite(payload.constData(), 1, static_cast<size_t>(payload.size()), out);
        fflush(out);
        return;
    }

#ifdef Q_OS_WIN
    OutputDebugStringW(reinterpret_cast<const wchar_t *>(QString(line + QLatin1Char('\n')).utf16()));
#endif
}

QString formatLine(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    const QString timestamp = QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss.zzz"));
    QString category;
    if (context.category && context.category[0] != '\0')
        category = QString::fromUtf8(context.category);
    else
        category = QStringLiteral("default");

    return QStringLiteral("%1 [%2] [%3] %4")
        .arg(timestamp, levelLabel(type), category, msg);
}

} // namespace

void AppLogger::install()
{
    QMutexLocker locker(&g_logMutex);
    if (g_installed)
        return;

    initConsoleLogging();
    g_minLevel = levelFromEnvironment();
    qInstallMessageHandler(messageHandler);
    g_installed = true;
}

void AppLogger::setMinLevel(Level level)
{
    QMutexLocker locker(&g_logMutex);
    g_minLevel = level;
}

AppLogger::Level AppLogger::minLevel()
{
    QMutexLocker locker(&g_logMutex);
    return g_minLevel;
}

QString AppLogger::logFilePath()
{
    return AppPaths::logsDir() + QLatin1Char('/') + logBaseName();
}

QString AppLogger::logsDir()
{
    return AppPaths::logsDir();
}

void AppLogger::logSessionStart(const QString &appVersion)
{
    const QString osName = QSysInfo::productType() + QLatin1Char(' ')
        + QSysInfo::productVersion();
    const QString cpuArch = QSysInfo::currentCpuArchitecture();
    const QString dataRoot = AppPaths::dataRoot();

    writeRawLine(QStringLiteral("========== session start =========="));
    writeRawLine(QStringLiteral("version: %1").arg(appVersion));
    writeRawLine(QStringLiteral("qt: %1").arg(QString::fromLatin1(qVersion())));
    writeRawLine(QStringLiteral("os: %1 (%2)").arg(osName, cpuArch));
    writeRawLine(QStringLiteral("dataRoot: %1").arg(dataRoot));
    writeRawLine(QStringLiteral("pid: %1").arg(QCoreApplication::applicationPid()));
}

void AppLogger::logSessionEnd()
{
    writeRawLine(QStringLiteral("========== session ended =========="));
}

void AppLogger::writeRawLine(const QString &line)
{
    QMutexLocker locker(&g_logMutex);
    appendToLogFile(line);
    writeToConsole(QtInfoMsg, line);
}

void AppLogger::writeCrashLine(const QString &line)
{
    appendToLogFileDirect(line);
    writeToConsole(QtFatalMsg, line);
}

AppLogger::Level AppLogger::levelFromEnvironment()
{
    const QString env = QProcessEnvironment::systemEnvironment()
                            .value(QStringLiteral("PIXELSTUDIO_LOG_LEVEL"));
    if (!env.isEmpty())
        return parseLevelToken(env);
    return defaultMinLevel();
}

void AppLogger::messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    AppLogger::Level minLevel = AppLogger::Level::Info;
    {
        QMutexLocker locker(&g_logMutex);
        minLevel = g_minLevel;
    }

    if (!levelAllows(minLevel, type))
        return;

    const QString line = formatLine(type, context, msg);
    {
        QMutexLocker locker(&g_logMutex);
        appendToLogFile(line);
    }
    writeToConsole(type, line);

    if (type == QtFatalMsg) {
        CrashHandler::captureFatalDump();
        abort();
    }
}
