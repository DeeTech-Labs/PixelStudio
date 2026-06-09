#ifndef DISPLAYRASTERIZER_H
#define DISPLAYRASTERIZER_H

#include <QImage>
#include <QVector>
#include "processing/DisplayCodeGenerator.h"
#include "processing/DisplayProfile.h"
#include "processing/ImageFiltersPipeline.h"

class DisplayRasterizer
{
public:
    struct Result {
        QImage preview;
        QImage processPreview;
        QByteArray monoBuffer;
        QVector<bool> monoBits;
        QVector<quint16> rgb565;
        QByteArray grayscale8;
        QByteArray rgb888;
        QByteArray rgb233;
        QVector<quint32> rgb24;
        QVector<quint32> indexedPalette;
        int width = 0;
        int height = 0;
    };

    static Result convert(const QImage &source,
                          int width,
                          int height,
                          DisplayProfile::ScaleMode scaleMode,
                          const ImageFiltersPipeline::Params &filters,
                          DisplayCodeGenerator::EncodingMode encodingMode,
                          int monoThreshold = 128,
                          bool invertMono = false,
                          bool linearColorSpace = true);

    static void invertMonoResult(Result &result);

    static void buildIndexedPalette(Result &result);

    static void applyPreviewEncoding(Result &result, DisplayCodeGenerator::EncodingMode encodingMode);
};

#endif // DISPLAYRASTERIZER_H
