#include "app/studio/controllers/image/ImageTransformController.h"
#include "app/studio/DisplayConverter.h"
#include "app/studio/model/ConverterState.h"
#include "app/studio/pipeline/ImagePipelineController.h"

#include <QImage>
#include <QSize>

namespace {

QSize fittedContentSize(const QImage &oriented, int tw, int th, DisplayProfile::ScaleMode mode)
{
    if (oriented.isNull() || tw < 1 || th < 1)
        return {};

    if (mode == DisplayProfile::Stretch || mode == DisplayProfile::Crop)
        return QSize(tw, th);

    const qreal sw = oriented.width();
    const qreal sh = oriented.height();
    if (sw < 1 || sh < 1)
        return {};
    const qreal scale = qMin(qreal(tw) / sw, qreal(th) / sh);
    return QSize(qMax(1, qRound(sw * scale)), qMax(1, qRound(sh * scale)));
}

} // namespace

ImageTransformController::ImageTransformController(QObject *parent)
    : QObject(parent)
{
}

void ImageTransformController::attach(DisplayConverter *host, ConverterState *state)
{
    m_host = host;
    m_state = state;
}

int ImageTransformController::scaleMode() const
{
    return m_state ? static_cast<int>(m_state->scaleMode) : 0;
}

int ImageTransformController::rotation() const
{
    return m_state ? m_state->rotation : 0;
}

bool ImageTransformController::flipHorizontal() const
{
    return m_state && m_state->flipHorizontal;
}

bool ImageTransformController::flipVertical() const
{
    return m_state && m_state->flipVertical;
}

int ImageTransformController::offsetX() const
{
    return m_state ? m_state->offsetX : 0;
}

int ImageTransformController::offsetY() const
{
    return m_state ? m_state->offsetY : 0;
}

bool ImageTransformController::invertMono() const
{
    return m_state && m_state->invertMono;
}

void ImageTransformController::requestRebuild()
{
    if (m_host)
        ImagePipelineController::scheduleRebuild(*m_host);
}

void ImageTransformController::onOrientationChanged()
{
    if (!m_host)
        return;
    m_host->markOrientedDirty();
    if (m_host->hasImage())
        m_host->notifySourceGeometryChanged();
    requestRebuild();
}

void ImageTransformController::setScaleMode(int mode)
{
    if (!m_state)
        return;
    const auto next = static_cast<DisplayProfile::ScaleMode>(qBound(0, mode, 2));
    if (m_state->scaleMode == next)
        return;
    m_state->scaleMode = next;
    emit scaleModeChanged();
    requestRebuild();
}

void ImageTransformController::setRotation(int degrees)
{
    if (!m_state)
        return;
    degrees = ((degrees % 360) + 360) % 360;
    if (degrees != 0 && degrees != 90 && degrees != 180 && degrees != 270)
        degrees = 0;
    if (m_state->rotation == degrees)
        return;
    m_state->rotation = degrees;
    emit rotationChanged();
    onOrientationChanged();
}

void ImageTransformController::rotateClockwise()
{
    setRotation((rotation() + 90) % 360);
}

void ImageTransformController::setFlipHorizontal(bool on)
{
    if (!m_state || m_state->flipHorizontal == on)
        return;
    m_state->flipHorizontal = on;
    emit flipHorizontalChanged();
    onOrientationChanged();
}

void ImageTransformController::setFlipVertical(bool on)
{
    if (!m_state || m_state->flipVertical == on)
        return;
    m_state->flipVertical = on;
    emit flipVerticalChanged();
    onOrientationChanged();
}

void ImageTransformController::setOffsetX(int value)
{
    if (!m_state || m_state->offsetX == value)
        return;
    m_state->offsetX = value;
    emit offsetChanged();
    if (m_host)
        m_host->schedulePersistSession();
}

void ImageTransformController::setOffsetY(int value)
{
    if (!m_state || m_state->offsetY == value)
        return;
    m_state->offsetY = value;
    emit offsetChanged();
    if (m_host)
        m_host->schedulePersistSession();
}

void ImageTransformController::setInvertMono(bool on)
{
    if (!m_state || m_state->invertMono == on)
        return;
    m_state->invertMono = on;
    emit invertMonoChanged();
    if (m_host && m_host->hasImage())
        requestRebuild();
}

void ImageTransformController::resetTransform()
{
    if (!m_state)
        return;
    m_state->rotation = 0;
    m_state->flipHorizontal = false;
    m_state->flipVertical = false;
    m_state->scaleMode = DisplayProfile::Fit;
    m_state->offsetX = 0;
    m_state->offsetY = 0;
    emit rotationChanged();
    emit flipHorizontalChanged();
    emit flipVerticalChanged();
    emit scaleModeChanged();
    emit offsetChanged();
    if (m_host)
        m_host->markOrientedDirty();
    requestRebuild();
}

void ImageTransformController::centerOffsetOnDisplay()
{
    if (!m_host || !m_state || !m_host->hasImage())
        return;
    const QSize content = fittedContentSize(m_host->orientedSource(),
                                            m_state->displayWidth,
                                            m_state->displayHeight,
                                            m_state->scaleMode);
    if (content.isEmpty())
        return;
    setOffsetX((m_state->displayWidth - content.width()) / 2);
    setOffsetY((m_state->displayHeight - content.height()) / 2);
}

void ImageTransformController::notifyAllChanged()
{
    emit scaleModeChanged();
    emit rotationChanged();
    emit flipHorizontalChanged();
    emit flipVerticalChanged();
    emit offsetChanged();
    emit invertMonoChanged();
}

void ImageTransformController::notifyOffsetChanged()
{
    emit offsetChanged();
}

void ImageTransformController::notifyInvertMonoChanged()
{
    emit invertMonoChanged();
}
