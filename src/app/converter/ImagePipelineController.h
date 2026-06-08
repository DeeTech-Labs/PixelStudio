#ifndef PIXELSTUDIO_APP_CONVERTER_IMAGEPIPELINECONTROLLER_H
#define PIXELSTUDIO_APP_CONVERTER_IMAGEPIPELINECONTROLLER_H

class DisplayConverter;

class ImagePipelineController
{
public:
    static void scheduleRebuild(DisplayConverter &converter, bool immediate = false);
    static void startAsyncRebuild(DisplayConverter &converter);
    static void onAsyncRebuildFinished(DisplayConverter &converter);
    static void updateCodePreview(DisplayConverter &converter);
    static void updateFlashReport(DisplayConverter &converter);
};

#endif // PIXELSTUDIO_APP_CONVERTER_IMAGEPIPELINECONTROLLER_H
