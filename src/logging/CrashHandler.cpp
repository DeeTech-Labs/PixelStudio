#include "logging/CrashHandler.h"

#include "logging/AppLogger.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>

#ifdef Q_OS_WIN
#include <windows.h>
#include <dbghelp.h>
#endif

namespace {

#ifdef Q_OS_WIN
constexpr int kMaxDumpFiles = 3;

LPTOP_LEVEL_EXCEPTION_FILTER g_previousFilter = nullptr;
bool g_installed = false;

QString dumpFileName(const QString &suffix = QString())
{
    const QString timestamp = QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_HHmmss"));
    if (suffix.isEmpty())
        return QStringLiteral("PixelStudio_%1.dmp").arg(timestamp);
    return QStringLiteral("PixelStudio_%1_%2.dmp").arg(timestamp, suffix);
}

void pruneOldDumps()
{
    QDir dir(AppLogger::logsDir());
    if (!dir.exists())
        return;

    QFileInfoList dumps = dir.entryInfoList(
        {QStringLiteral("PixelStudio_*.dmp")},
        QDir::Files,
        QDir::Time);
    while (dumps.size() > kMaxDumpFiles)
        QFile::remove(dumps.takeFirst().absoluteFilePath());
}

bool writeMiniDump(EXCEPTION_POINTERS *exceptionPointers, const QString &dumpPath)
{
    HANDLE file = CreateFileW(reinterpret_cast<LPCWSTR>(dumpPath.utf16()),
                              GENERIC_WRITE,
                              FILE_SHARE_READ,
                              nullptr,
                              CREATE_ALWAYS,
                              FILE_ATTRIBUTE_NORMAL,
                              nullptr);
    if (file == INVALID_HANDLE_VALUE)
        return false;

    MINIDUMP_EXCEPTION_INFORMATION exceptionInfo;
    exceptionInfo.ThreadId = GetCurrentThreadId();
    exceptionInfo.ExceptionPointers = exceptionPointers;
    exceptionInfo.ClientPointers = FALSE;

    const MINIDUMP_TYPE dumpType = static_cast<MINIDUMP_TYPE>(
        MiniDumpWithFullMemory | MiniDumpWithHandleData | MiniDumpWithUnloadedModules
        | MiniDumpWithThreadInfo | MiniDumpWithProcessThreadData);

    const BOOL ok = MiniDumpWriteDump(GetCurrentProcess(),
                                      GetCurrentProcessId(),
                                      file,
                                      dumpType,
                                      exceptionPointers ? &exceptionInfo : nullptr,
                                      nullptr,
                                      nullptr);
    FlushFileBuffers(file);
    CloseHandle(file);
    return ok == TRUE;
}

bool captureDump(EXCEPTION_POINTERS *exceptionPointers, const QString &suffix)
{
    const QString dumpPath = AppLogger::logsDir() + QLatin1Char('/') + dumpFileName(suffix);
    if (!writeMiniDump(exceptionPointers, dumpPath))
        return false;

    AppLogger::writeCrashLine(QStringLiteral("minidump: %1").arg(dumpPath));
    pruneOldDumps();
    return true;
}

LONG WINAPI unhandledExceptionFilter(EXCEPTION_POINTERS *exceptionPointers)
{
    const DWORD code = exceptionPointers && exceptionPointers->ExceptionRecord
        ? exceptionPointers->ExceptionRecord->ExceptionCode
        : 0;
    const void *address = exceptionPointers && exceptionPointers->ExceptionRecord
        ? exceptionPointers->ExceptionRecord->ExceptionAddress
        : nullptr;

    AppLogger::writeCrashLine(QStringLiteral("========== unhandled exception =========="));
    AppLogger::writeCrashLine(QStringLiteral("exceptionCode: 0x%1")
                                  .arg(QString::number(code, 16)));
    AppLogger::writeCrashLine(QStringLiteral("exceptionAddress: 0x%1")
                                  .arg(QString::number(reinterpret_cast<quintptr>(address), 16)));

    if (!captureDump(exceptionPointers, QString())) {
        const QString dumpPath = AppLogger::logsDir() + QLatin1Char('/') + dumpFileName();
        AppLogger::writeCrashLine(QStringLiteral("minidump: failed to write %1").arg(dumpPath));
    }

    if (g_previousFilter)
        return g_previousFilter(exceptionPointers);
    return EXCEPTION_EXECUTE_HANDLER;
}
#endif

} // namespace

void CrashHandler::install()
{
#ifdef Q_OS_WIN
    if (g_installed)
        return;
    g_previousFilter = SetUnhandledExceptionFilter(unhandledExceptionFilter);
    g_installed = true;
#endif
}

void CrashHandler::uninstall()
{
#ifdef Q_OS_WIN
    if (!g_installed)
        return;
    SetUnhandledExceptionFilter(g_previousFilter);
    g_previousFilter = nullptr;
    g_installed = false;
#endif
}

void CrashHandler::captureFatalDump()
{
#ifdef Q_OS_WIN
    AppLogger::writeCrashLine(QStringLiteral("========== fatal error =========="));
    if (!captureDump(nullptr, QStringLiteral("fatal"))) {
        const QString dumpPath = AppLogger::logsDir() + QLatin1Char('/')
            + dumpFileName(QStringLiteral("fatal"));
        AppLogger::writeCrashLine(QStringLiteral("minidump: failed to write %1").arg(dumpPath));
    }
#endif
}
