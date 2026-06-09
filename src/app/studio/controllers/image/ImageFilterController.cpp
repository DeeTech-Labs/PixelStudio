#include "app/studio/controllers/image/ImageFilterController.h"
#include "app/studio/DisplayConverter.h"
#include "app/studio/model/ConverterState.h"
#include "app/studio/pipeline/ImagePipelineController.h"

ImageFilterController::ImageFilterController(QObject *parent)
    : QObject(parent)
{
}

void ImageFilterController::attach(DisplayConverter *host, ConverterState *state)
{
    m_host = host;
    m_state = state;
}

bool ImageFilterController::dithering() const
{
    return m_state && m_state->dithering;
}

bool ImageFilterController::blackBackground() const
{
    return m_state && m_state->filterParams.blackBackground;
}

int ImageFilterController::brightness() const
{
    return m_state ? m_state->filterParams.brightness : 100;
}

int ImageFilterController::contrast() const
{
    return m_state ? m_state->filterParams.contrast : 100;
}

int ImageFilterController::saturation() const
{
    return m_state ? m_state->filterParams.saturation : 100;
}

int ImageFilterController::exposure() const
{
    return m_state ? m_state->filterParams.exposure : 100;
}

int ImageFilterController::gamma() const
{
    return m_state ? m_state->filterParams.gamma : 100;
}

int ImageFilterController::blur() const
{
    return m_state ? m_state->filterParams.blur : 0;
}

int ImageFilterController::posterizeRgb() const
{
    return m_state ? m_state->filterParams.posterizeRgb : 0;
}

bool ImageFilterController::colorMaskEnabled() const
{
    return m_state && m_state->filterParams.colorMaskEnabled;
}

QColor ImageFilterController::maskColor() const
{
    return m_state ? m_state->filterParams.maskColor : QColor(Qt::black);
}

int ImageFilterController::maskTolerance() const
{
    return m_state ? m_state->filterParams.maskTolerance : 0;
}

int ImageFilterController::maskAmplify() const
{
    return m_state ? m_state->filterParams.maskAmplify : 1;
}

bool ImageFilterController::sharpen() const
{
    return m_state && m_state->filterParams.sharpen;
}

int ImageFilterController::sobelEdges() const
{
    return m_state ? m_state->filterParams.sobelEdges : 0;
}

int ImageFilterController::posterizeGray() const
{
    return m_state ? m_state->filterParams.posterizeGray : 0;
}

int ImageFilterController::ditherMode() const
{
    return m_state ? static_cast<int>(m_state->filterParams.ditherMode) : 0;
}

int ImageFilterController::contourMode() const
{
    return m_state ? static_cast<int>(m_state->filterParams.contourMode) : 0;
}

int ImageFilterController::tonePreset() const
{
    return m_state ? static_cast<int>(m_state->filterParams.tonePreset) : 0;
}

bool ImageFilterController::filterInvert() const
{
    return m_state && m_state->filterParams.invert;
}

void ImageFilterController::requestRebuild()
{
    if (m_host)
        ImagePipelineController::scheduleRebuild(*m_host);
}

void ImageFilterController::markToneCustom()
{
    if (!m_state)
        return;
    if (m_state->filterParams.tonePreset == ImageFiltersPipeline::TonePreset::Custom)
        return;
    m_state->filterParams.tonePreset = ImageFiltersPipeline::TonePreset::Custom;
    emit tonePresetChanged();
}

void ImageFilterController::setDithering(bool on)
{
    if (!m_state)
        return;
    const auto mode = on ? ImageFiltersPipeline::DitherMode::FloydSteinberg
                         : ImageFiltersPipeline::DitherMode::None;
    if (m_state->dithering == on && m_state->filterParams.ditherMode == mode)
        return;
    m_state->dithering = on;
    m_state->filterParams.ditherMode = mode;
    emit ditheringChanged();
    emit ditherModeChanged();
    requestRebuild();
}

void ImageFilterController::setBlackBackground(bool on)
{
    if (!m_state || m_state->filterParams.blackBackground == on)
        return;
    m_state->filterParams.blackBackground = on;
    emit blackBackgroundChanged();
    requestRebuild();
}

void ImageFilterController::setBrightness(int value)
{
    if (!m_state)
        return;
    value = qBound(0, value, 200);
    if (m_state->filterParams.brightness == value)
        return;
    m_state->filterParams.brightness = value;
    markToneCustom();
    emit brightnessChanged();
    requestRebuild();
}

void ImageFilterController::setContrast(int value)
{
    if (!m_state)
        return;
    value = qBound(0, value, 200);
    if (m_state->filterParams.contrast == value)
        return;
    m_state->filterParams.contrast = value;
    markToneCustom();
    emit contrastChanged();
    requestRebuild();
}

void ImageFilterController::setSaturation(int value)
{
    if (!m_state)
        return;
    value = qBound(0, value, 200);
    if (m_state->filterParams.saturation == value)
        return;
    m_state->filterParams.saturation = value;
    markToneCustom();
    emit saturationChanged();
    requestRebuild();
}

void ImageFilterController::setExposure(int value)
{
    if (!m_state)
        return;
    value = qBound(50, value, 200);
    if (m_state->filterParams.exposure == value)
        return;
    m_state->filterParams.exposure = value;
    markToneCustom();
    emit exposureChanged();
    requestRebuild();
}

void ImageFilterController::setGamma(int value)
{
    if (!m_state)
        return;
    value = qBound(50, value, 200);
    if (m_state->filterParams.gamma == value)
        return;
    m_state->filterParams.gamma = value;
    markToneCustom();
    emit gammaChanged();
    requestRebuild();
}

void ImageFilterController::setBlur(int value)
{
    if (!m_state)
        return;
    value = qBound(0, value, 6);
    if (m_state->filterParams.blur == value)
        return;
    m_state->filterParams.blur = value;
    emit blurChanged();
    requestRebuild();
}

void ImageFilterController::setPosterizeRgb(int value)
{
    if (!m_state)
        return;
    value = qBound(0, value, 30);
    if (m_state->filterParams.posterizeRgb == value)
        return;
    m_state->filterParams.posterizeRgb = value;
    emit posterizeRgbChanged();
    requestRebuild();
}

void ImageFilterController::setColorMaskEnabled(bool on)
{
    if (!m_state || m_state->filterParams.colorMaskEnabled == on)
        return;
    m_state->filterParams.colorMaskEnabled = on;
    emit colorMaskEnabledChanged();
    requestRebuild();
}

void ImageFilterController::setMaskColor(const QColor &color)
{
    if (!m_state)
        return;
    const QColor safe = color.isValid() ? color : QColor(Qt::black);
    if (m_state->filterParams.maskColor == safe)
        return;
    m_state->filterParams.maskColor = safe;
    emit maskColorChanged();
    requestRebuild();
}

void ImageFilterController::setMaskTolerance(int value)
{
    if (!m_state)
        return;
    value = qBound(0, value, 255);
    if (m_state->filterParams.maskTolerance == value)
        return;
    m_state->filterParams.maskTolerance = value;
    emit maskToleranceChanged();
    requestRebuild();
}

void ImageFilterController::setMaskAmplify(int value)
{
    if (!m_state)
        return;
    value = qBound(1, value, 10);
    if (m_state->filterParams.maskAmplify == value)
        return;
    m_state->filterParams.maskAmplify = value;
    emit maskAmplifyChanged();
    requestRebuild();
}

void ImageFilterController::setSharpen(bool on)
{
    if (!m_state || m_state->filterParams.sharpen == on)
        return;
    m_state->filterParams.sharpen = on;
    emit sharpenChanged();
    requestRebuild();
}

void ImageFilterController::setSobelEdges(int value)
{
    if (!m_state)
        return;
    value = qBound(0, value, 100);
    if (m_state->filterParams.sobelEdges == value)
        return;
    m_state->filterParams.sobelEdges = value;
    emit sobelEdgesChanged();
    requestRebuild();
}

void ImageFilterController::setPosterizeGray(int value)
{
    if (!m_state)
        return;
    value = qBound(0, value, 30);
    if (m_state->filterParams.posterizeGray == value)
        return;
    m_state->filterParams.posterizeGray = value;
    emit posterizeGrayChanged();
    requestRebuild();
}

void ImageFilterController::setDitherMode(int mode)
{
    if (!m_state)
        return;
    const auto next = mode == static_cast<int>(ImageFiltersPipeline::DitherMode::FloydSteinberg)
        ? ImageFiltersPipeline::DitherMode::FloydSteinberg
        : (mode == static_cast<int>(ImageFiltersPipeline::DitherMode::Jjn)
            ? ImageFiltersPipeline::DitherMode::Jjn
            : (mode == static_cast<int>(ImageFiltersPipeline::DitherMode::Bayer)
                ? ImageFiltersPipeline::DitherMode::Bayer
                : ImageFiltersPipeline::DitherMode::None));
    if (m_state->filterParams.ditherMode == next)
        return;
    m_state->filterParams.ditherMode = next;
    const bool dithering = next == ImageFiltersPipeline::DitherMode::FloydSteinberg;
    if (m_state->dithering != dithering) {
        m_state->dithering = dithering;
        emit ditheringChanged();
    }
    emit ditherModeChanged();
    requestRebuild();
}

void ImageFilterController::setContourMode(int mode)
{
    if (!m_state)
        return;
    const auto next = mode == static_cast<int>(ImageFiltersPipeline::ContourMode::FourDir)
        ? ImageFiltersPipeline::ContourMode::FourDir
        : (mode == static_cast<int>(ImageFiltersPipeline::ContourMode::EightDir)
            ? ImageFiltersPipeline::ContourMode::EightDir
            : ImageFiltersPipeline::ContourMode::None);
    if (m_state->filterParams.contourMode == next)
        return;
    m_state->filterParams.contourMode = next;
    emit contourModeChanged();
    requestRebuild();
}

void ImageFilterController::setTonePreset(int preset)
{
    if (!m_state)
        return;
    const auto next = static_cast<ImageFiltersPipeline::TonePreset>(qBound(0, preset, 2));
    const ImageFiltersPipeline::Params tone = ImageFiltersPipeline::paramsForTonePreset(next);
    m_state->filterParams.tonePreset = next;
    if (next != ImageFiltersPipeline::TonePreset::Custom) {
        m_state->filterParams.brightness = tone.brightness;
        m_state->filterParams.contrast = tone.contrast;
        m_state->filterParams.saturation = tone.saturation;
        m_state->filterParams.exposure = tone.exposure;
        m_state->filterParams.gamma = tone.gamma;
        m_state->filterParams.posterizeGray = tone.posterizeGray;
    }
    emit brightnessChanged();
    emit contrastChanged();
    emit saturationChanged();
    emit exposureChanged();
    emit gammaChanged();
    emit posterizeGrayChanged();
    emit tonePresetChanged();
    requestRebuild();
}

void ImageFilterController::setFilterInvert(bool on)
{
    if (!m_state || m_state->filterParams.invert == on)
        return;
    m_state->filterParams.invert = on;
    emit filterInvertChanged();
    if (m_host && m_host->hasImage())
        requestRebuild();
}

void ImageFilterController::resetFilters()
{
    FilterResetSideEffects effects;
    if (!m_state)
        return;

    const int threshold = m_state->monoThreshold;
    const bool invert = m_state->invertMono;
    m_state->filterParams = ImageFiltersPipeline::Params{};
    m_state->filterParams.threshold = threshold;
    effects.ditheringChanged = m_state->dithering != true;
    m_state->dithering = true;
    m_state->filterParams.ditherMode = ImageFiltersPipeline::DitherMode::FloydSteinberg;
    effects.invertMonoChanged = m_state->invertMono != invert;
    m_state->invertMono = invert;
    m_state->filterParams.invert = false;
    emit ditheringChanged();
    emit blackBackgroundChanged();
    emit brightnessChanged();
    emit contrastChanged();
    emit saturationChanged();
    emit exposureChanged();
    emit gammaChanged();
    emit blurChanged();
    emit posterizeRgbChanged();
    emit colorMaskEnabledChanged();
    emit maskColorChanged();
    emit maskToleranceChanged();
    emit maskAmplifyChanged();
    emit sharpenChanged();
    emit sobelEdgesChanged();
    emit posterizeGrayChanged();
    emit ditherModeChanged();
    emit contourModeChanged();
    emit filterInvertChanged();
    emit tonePresetChanged();
    if (effects.invertMonoChanged)
        emit hostInvertMonoChanged();
    requestRebuild();
}

void ImageFilterController::syncFromState()
{
    emit ditheringChanged();
    emit blackBackgroundChanged();
    emit brightnessChanged();
    emit contrastChanged();
    emit saturationChanged();
    emit exposureChanged();
    emit gammaChanged();
    emit blurChanged();
    emit posterizeRgbChanged();
    emit colorMaskEnabledChanged();
    emit maskColorChanged();
    emit maskToleranceChanged();
    emit maskAmplifyChanged();
    emit sharpenChanged();
    emit sobelEdgesChanged();
    emit posterizeGrayChanged();
    emit ditherModeChanged();
    emit contourModeChanged();
    emit filterInvertChanged();
    emit tonePresetChanged();
}

void ImageFilterController::syncThreshold(int monoThreshold)
{
    if (m_state)
        m_state->filterParams.threshold = monoThreshold;
}
