#ifndef PIXELSTUDIO_LOGGING_APPLOGGER_H
#define PIXELSTUDIO_LOGGING_APPLOGGER_H

#include <QString>

#include <QLoggingCategory>

class AppLogger
{
public:
    enum class Level {
        Debug,
        Info,
        Warning,
        Error,
        Fatal,
    };

    static void install();
    static void setMinLevel(Level level);
    static Level minLevel();

    static QString logFilePath();
    static QString logsDir();

    static void logSessionStart(const QString &appVersion);
    static void logSessionEnd();

    static void writeRawLine(const QString &line);

    // Mutex-free append for crash/fatal paths (no rotation).
    static void writeCrashLine(const QString &line);

private:
    static Level levelFromEnvironment();
    static void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
};

#endif // PIXELSTUDIO_LOGGING_APPLOGGER_H
