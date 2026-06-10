#include "logging/CrashHandler.h"

#include "logging/AppLogger.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

#ifdef Q_OS_WIN
#include <windows.h>
#include <dbghelp.h>
#endif

#if defined(Q_OS_MACOS)
#include <execinfo.h>
#include <signal.h>
#include <unistd.h>
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
#endif // Q_OS_WIN

#if defined(Q_OS_MACOS)
constexpr int kMaxCrashReports = 3;
constexpr int kMaxBacktraceFrames = 64;

struct sigaction g_previousHandlers[NSIG] = {};
bool g_installed = false;

QString crashReportFileName(const QString &suffix = QString())
{
    const QString timestamp = QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_HHmmss"));
    if (suffix.isEmpty())
        return QStringLiteral("PixelStudio_%1.crash").arg(timestamp);
    return QStringLiteral("PixelStudio_%1_%2.crash").arg(timestamp, suffix);
}

void pruneOldCrashReports()
{
    QDir dir(AppLogger::logsDir());
    if (!dir.exists())
        return;

    QFileInfoList reports = dir.entryInfoList(
        {QStringLiteral("PixelStudio_*.crash")},
        QDir::Files,
        QDir::Time);
    while (reports.size() > kMaxCrashReports)
        QFile::remove(reports.takeFirst().absoluteFilePath());
}

void writeBacktraceLines()
{
    void *frames[kMaxBacktraceFrames];
    const int count = backtrace(frames, kMaxBacktraceFrames);
    char **symbols = backtrace_symbols(frames, count);
    if (!symbols)
        return;

    for (int i = 0; i < count; ++i)
        AppLogger::writeCrashLine(QString::fromLocal8Bit(symbols[i]));
    free(symbols);
}

bool writeCrashReport(int signum, void *faultAddress, const QString &suffix)
{
    const QString reportPath = AppLogger::logsDir() + QLatin1Char('/') + crashReportFileName(suffix);

    QFile file(reportPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
        return false;

    QTextStream stream(&file);
    stream << QStringLiteral("========== crash report ==========\n");
    if (signum != 0)
        stream << QStringLiteral("signal: %1\n").arg(signum);
    if (faultAddress) {
        stream << QStringLiteral("faultAddress: 0x%1\n")
                      .arg(QString::number(reinterpret_cast<quintptr>(faultAddress), 16));
    }

    void *frames[kMaxBacktraceFrames];
    const int count = backtrace(frames, kMaxBacktraceFrames);
    char **symbols = backtrace_symbols(frames, count);
    if (symbols) {
        for (int i = 0; i < count; ++i)
            stream << QString::fromLocal8Bit(symbols[i]) << QLatin1Char('\n');
        free(symbols);
    }
    stream.flush();
    file.close();

    AppLogger::writeCrashLine(QStringLiteral("crash report: %1").arg(reportPath));
    pruneOldCrashReports();
    return true;
}

void fatalSignalHandler(int signum, siginfo_t *info, void * /*context*/)
{
    void *faultAddress = info ? info->si_addr : nullptr;

    AppLogger::writeCrashLine(QStringLiteral("========== fatal signal =========="));
    AppLogger::writeCrashLine(QStringLiteral("signal: %1").arg(signum));
    if (faultAddress) {
        AppLogger::writeCrashLine(QStringLiteral("faultAddress: 0x%1")
                                      .arg(QString::number(reinterpret_cast<quintptr>(faultAddress),
                                                           16)));
    }
    writeBacktraceLines();

    if (!writeCrashReport(signum, faultAddress, QString())) {
        const QString reportPath = AppLogger::logsDir() + QLatin1Char('/') + crashReportFileName();
        AppLogger::writeCrashLine(QStringLiteral("crash report: failed to write %1").arg(reportPath));
    }

    struct sigaction previous = g_previousHandlers[signum];
    sigaction(signum, &previous, nullptr);
    raise(signum);
}
#endif // Q_OS_MACOS

} // namespace

void CrashHandler::install()
{
#ifdef Q_OS_WIN
    if (g_installed)
        return;
    g_previousFilter = SetUnhandledExceptionFilter(unhandledExceptionFilter);
    g_installed = true;
#elif defined(Q_OS_MACOS)
    if (g_installed)
        return;

    struct sigaction action;
    action.sa_sigaction = fatalSignalHandler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_SIGINFO;

    const int signals[] = {SIGSEGV, SIGABRT, SIGBUS, SIGILL, SIGFPE};
    for (int signum : signals) {
        if (sigaction(signum, &action, &g_previousHandlers[signum]) != 0)
            continue;
    }
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
#elif defined(Q_OS_MACOS)
    if (!g_installed)
        return;

    const int signals[] = {SIGSEGV, SIGABRT, SIGBUS, SIGILL, SIGFPE};
    for (int signum : signals)
        sigaction(signum, &g_previousHandlers[signum], nullptr);
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
#elif defined(Q_OS_MACOS)
    AppLogger::writeCrashLine(QStringLiteral("========== fatal error =========="));
    writeBacktraceLines();
    if (!writeCrashReport(0, nullptr, QStringLiteral("fatal"))) {
        const QString reportPath = AppLogger::logsDir() + QLatin1Char('/')
            + crashReportFileName(QStringLiteral("fatal"));
        AppLogger::writeCrashLine(QStringLiteral("crash report: failed to write %1").arg(reportPath));
    }
#endif
}
