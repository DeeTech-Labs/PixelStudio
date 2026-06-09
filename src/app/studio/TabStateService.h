#ifndef PIXELSTUDIO_APP_STUDIO_TABSTATESERVICE_H
#define PIXELSTUDIO_APP_STUDIO_TABSTATESERVICE_H

#include <QImage>

#include "app/studio/model/ConverterTabSnapshot.h"

class DisplayConverter;
class PreviewImageProvider;

class TabStateService
{
public:
    static ConverterTabSnapshot capture(const DisplayConverter &converter, const QString &tabId);
    static void restore(DisplayConverter &converter, const ConverterTabSnapshot &snapshot);
    static void publishSnapshotPreviews(PreviewImageProvider *provider,
                                        ConverterTabSnapshot *snapshot,
                                        const QImage &orientedSource = {});
};

#endif // PIXELSTUDIO_APP_STUDIO_TABSTATESERVICE_H
