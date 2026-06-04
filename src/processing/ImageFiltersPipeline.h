#ifndef IMAGEFILTERSPIPELINE_H
#define IMAGEFILTERSPIPELINE_H

#include <QColor>
#include <QImage>

class ImageFiltersPipeline
{
public:
    enum class DitherMode {
        None = 0,
        FloydSteinberg = 1,
        Jjn = 2,
        Bayer = 3
    };

    enum class ContourMode {
        None = 0,
        FourDir = 1,
        EightDir = 2
    };

    enum class TonePreset {
        Custom = 0,
        Icon = 1,
        Photo = 2
    };

    enum class OutputMode {
        Mono1Bit = 0,
        Grayscale8 = 1,
        Color = 2
    };

    struct Params {
        bool blackBackground = false;
        int brightness = 100;
        int contrast = 100;
        int saturation = 100;
        int exposure = 100;
        int gamma = 100;
        int blur = 0;
        int posterizeRgb = 0;
        bool colorMaskEnabled = false;
        QColor maskColor = QColor(Qt::black);
        int maskTolerance = 0;
        int maskAmplify = 1;
        bool sharpen = false;
        int sobelEdges = 0;
        int posterizeGray = 0;
        DitherMode ditherMode = DitherMode::None;
        bool thresholdEnabled = false;
        int threshold = 128;
        ContourMode contourMode = ContourMode::None;
        bool invert = false;
        TonePreset tonePreset = TonePreset::Custom;
        OutputMode outputMode = OutputMode::Color;

        bool usesGrayPipeline() const;
    };

    static Params paramsForTonePreset(TonePreset preset);
    static QImage apply(const QImage &source, const Params &params);
};

#endif // IMAGEFILTERSPIPELINE_H
