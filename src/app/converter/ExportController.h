#ifndef PIXELSTUDIO_APP_CONVERTER_EXPORTCONTROLLER_H
#define PIXELSTUDIO_APP_CONVERTER_EXPORTCONTROLLER_H

#include <QUrl>
#include <QVariantList>

class DisplayConverter;

class ExportController
{
public:
    static bool buildSpriteAtlas(DisplayConverter &converter,
                                 const QVariantList &urls,
                                 const QUrl &targetFile,
                                 int frameWidth,
                                 int frameHeight);
    static bool saveCodeToFile(DisplayConverter &converter, const QUrl &url);
    static bool saveBinaryToFile(DisplayConverter &converter, const QUrl &url);
    static void enqueueBatchCodeExport(DisplayConverter &converter,
                                       const QVariantList &urls,
                                       const QUrl &targetFile);
    static void configureWatchFolder(DisplayConverter &converter,
                                     const QString &inputFolder,
                                     const QString &outputFolder);
    static void setWatchFolderActive(DisplayConverter &converter, bool active);
};

#endif // PIXELSTUDIO_APP_CONVERTER_EXPORTCONTROLLER_H
