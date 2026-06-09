#ifndef PIXELSTUDIO_APP_CONVERTER_IMAGEFILTERCONTROLLER_H
#define PIXELSTUDIO_APP_CONVERTER_IMAGEFILTERCONTROLLER_H

#include <QColor>
#include <QObject>

#include "processing/ImageFiltersPipeline.h"

class DisplayConverter;
struct ConverterState;

struct FilterResetSideEffects
{
    bool ditheringChanged = false;
    bool invertMonoChanged = false;
};

class ImageFilterController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool dithering READ dithering WRITE setDithering NOTIFY ditheringChanged)
    Q_PROPERTY(bool blackBackground READ blackBackground WRITE setBlackBackground NOTIFY blackBackgroundChanged)
    Q_PROPERTY(int brightness READ brightness WRITE setBrightness NOTIFY brightnessChanged)
    Q_PROPERTY(int contrast READ contrast WRITE setContrast NOTIFY contrastChanged)
    Q_PROPERTY(int saturation READ saturation WRITE setSaturation NOTIFY saturationChanged)
    Q_PROPERTY(int exposure READ exposure WRITE setExposure NOTIFY exposureChanged)
    Q_PROPERTY(int gamma READ gamma WRITE setGamma NOTIFY gammaChanged)
    Q_PROPERTY(int blur READ blur WRITE setBlur NOTIFY blurChanged)
    Q_PROPERTY(int posterizeRgb READ posterizeRgb WRITE setPosterizeRgb NOTIFY posterizeRgbChanged)
    Q_PROPERTY(bool colorMaskEnabled READ colorMaskEnabled WRITE setColorMaskEnabled NOTIFY colorMaskEnabledChanged)
    Q_PROPERTY(QColor maskColor READ maskColor WRITE setMaskColor NOTIFY maskColorChanged)
    Q_PROPERTY(int maskTolerance READ maskTolerance WRITE setMaskTolerance NOTIFY maskToleranceChanged)
    Q_PROPERTY(int maskAmplify READ maskAmplify WRITE setMaskAmplify NOTIFY maskAmplifyChanged)
    Q_PROPERTY(bool sharpen READ sharpen WRITE setSharpen NOTIFY sharpenChanged)
    Q_PROPERTY(int sobelEdges READ sobelEdges WRITE setSobelEdges NOTIFY sobelEdgesChanged)
    Q_PROPERTY(int posterizeGray READ posterizeGray WRITE setPosterizeGray NOTIFY posterizeGrayChanged)
    Q_PROPERTY(int ditherMode READ ditherMode WRITE setDitherMode NOTIFY ditherModeChanged)
    Q_PROPERTY(int contourMode READ contourMode WRITE setContourMode NOTIFY contourModeChanged)
    Q_PROPERTY(int tonePreset READ tonePreset WRITE setTonePreset NOTIFY tonePresetChanged)
    Q_PROPERTY(bool filterInvert READ filterInvert WRITE setFilterInvert NOTIFY filterInvertChanged)

public:
    explicit ImageFilterController(QObject *parent = nullptr);

    void attach(DisplayConverter *host, ConverterState *state);

    bool dithering() const;
    bool blackBackground() const;
    int brightness() const;
    int contrast() const;
    int saturation() const;
    int exposure() const;
    int gamma() const;
    int blur() const;
    int posterizeRgb() const;
    bool colorMaskEnabled() const;
    QColor maskColor() const;
    int maskTolerance() const;
    int maskAmplify() const;
    bool sharpen() const;
    int sobelEdges() const;
    int posterizeGray() const;
    int ditherMode() const;
    int contourMode() const;
    int tonePreset() const;
    bool filterInvert() const;

    Q_INVOKABLE void setDithering(bool on);
    Q_INVOKABLE void setBlackBackground(bool on);
    Q_INVOKABLE void setBrightness(int value);
    Q_INVOKABLE void setContrast(int value);
    Q_INVOKABLE void setSaturation(int value);
    Q_INVOKABLE void setExposure(int value);
    Q_INVOKABLE void setGamma(int value);
    Q_INVOKABLE void setBlur(int value);
    Q_INVOKABLE void setPosterizeRgb(int value);
    Q_INVOKABLE void setColorMaskEnabled(bool on);
    Q_INVOKABLE void setMaskColor(const QColor &color);
    Q_INVOKABLE void setMaskTolerance(int value);
    Q_INVOKABLE void setMaskAmplify(int value);
    Q_INVOKABLE void setSharpen(bool on);
    Q_INVOKABLE void setSobelEdges(int value);
    Q_INVOKABLE void setPosterizeGray(int value);
    Q_INVOKABLE void setDitherMode(int mode);
    Q_INVOKABLE void setContourMode(int mode);
    Q_INVOKABLE void setTonePreset(int preset);
    Q_INVOKABLE void setFilterInvert(bool on);
    Q_INVOKABLE void resetFilters();

    void syncFromState();
    void syncThreshold(int monoThreshold);

signals:
    void ditheringChanged();
    void blackBackgroundChanged();
    void brightnessChanged();
    void contrastChanged();
    void saturationChanged();
    void exposureChanged();
    void gammaChanged();
    void blurChanged();
    void posterizeRgbChanged();
    void colorMaskEnabledChanged();
    void maskColorChanged();
    void maskToleranceChanged();
    void maskAmplifyChanged();
    void sharpenChanged();
    void sobelEdgesChanged();
    void posterizeGrayChanged();
    void ditherModeChanged();
    void contourModeChanged();
    void tonePresetChanged();
    void filterInvertChanged();
    void hostInvertMonoChanged();

private:
    void markToneCustom();
    void requestRebuild();

    DisplayConverter *m_host = nullptr;
    ConverterState *m_state = nullptr;
};

#endif // PIXELSTUDIO_APP_CONVERTER_IMAGEFILTERCONTROLLER_H
