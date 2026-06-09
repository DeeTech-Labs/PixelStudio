#ifndef PIXELSTUDIO_APP_CONVERTER_IMAGEPIPELINECONTROLLER_H
#define PIXELSTUDIO_APP_CONVERTER_IMAGEPIPELINECONTROLLER_H

struct ConverterState;
class DisplayConverter;

class ImagePipelineController
{
public:
    static void scheduleRebuild(DisplayConverter &converter, bool immediate = false);
    static void startAsyncRebuild(DisplayConverter &converter);
    static void onAsyncRebuildFinished(DisplayConverter &converter);
    static void updateCodePreview(ConverterState &state, DisplayConverter &converter);
    static void updateFlashReport(ConverterState &state, DisplayConverter &converter);
};

#endif // PIXELSTUDIO_APP_CONVERTER_IMAGEPIPELINECONTROLLER_H
