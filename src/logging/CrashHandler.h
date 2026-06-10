#ifndef PIXELSTUDIO_LOGGING_CRASHHANDLER_H
#define PIXELSTUDIO_LOGGING_CRASHHANDLER_H

class CrashHandler
{
public:
    static void install();
    static void uninstall();

    // Writes a minidump without SEH context (e.g. qFatal).
    static void captureFatalDump();
};

#endif // PIXELSTUDIO_LOGGING_CRASHHANDLER_H
