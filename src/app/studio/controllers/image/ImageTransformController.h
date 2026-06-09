#ifndef PIXELSTUDIO_APP_CONVERTER_IMAGETRANSFORMCONTROLLER_H
#define PIXELSTUDIO_APP_CONVERTER_IMAGETRANSFORMCONTROLLER_H

#include <QObject>

#include "processing/DisplayProfile.h"

class DisplayConverter;
struct ConverterState;

class ImageTransformController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int scaleMode READ scaleMode WRITE setScaleMode NOTIFY scaleModeChanged)
    Q_PROPERTY(int rotation READ rotation WRITE setRotation NOTIFY rotationChanged)
    Q_PROPERTY(bool flipHorizontal READ flipHorizontal WRITE setFlipHorizontal NOTIFY flipHorizontalChanged)
    Q_PROPERTY(bool flipVertical READ flipVertical WRITE setFlipVertical NOTIFY flipVerticalChanged)
    Q_PROPERTY(int offsetX READ offsetX WRITE setOffsetX NOTIFY offsetChanged)
    Q_PROPERTY(int offsetY READ offsetY WRITE setOffsetY NOTIFY offsetChanged)
    Q_PROPERTY(bool invertMono READ invertMono WRITE setInvertMono NOTIFY invertMonoChanged)

public:
    explicit ImageTransformController(QObject *parent = nullptr);

    void attach(DisplayConverter *host, ConverterState *state);

    int scaleMode() const;
    int rotation() const;
    bool flipHorizontal() const;
    bool flipVertical() const;
    int offsetX() const;
    int offsetY() const;
    bool invertMono() const;

    Q_INVOKABLE void setScaleMode(int mode);
    Q_INVOKABLE void setRotation(int degrees);
    Q_INVOKABLE void rotateClockwise();
    Q_INVOKABLE void setFlipHorizontal(bool on);
    Q_INVOKABLE void setFlipVertical(bool on);
    Q_INVOKABLE void setOffsetX(int value);
    Q_INVOKABLE void setOffsetY(int value);
    Q_INVOKABLE void setInvertMono(bool on);
    Q_INVOKABLE void resetTransform();
    Q_INVOKABLE void centerOffsetOnDisplay();

    void notifyAllChanged();
    void notifyOffsetChanged();
    void notifyInvertMonoChanged();

signals:
    void scaleModeChanged();
    void rotationChanged();
    void flipHorizontalChanged();
    void flipVerticalChanged();
    void offsetChanged();
    void invertMonoChanged();

private:
    void requestRebuild();
    void onOrientationChanged();

    DisplayConverter *m_host = nullptr;
    ConverterState *m_state = nullptr;
};

#endif // PIXELSTUDIO_APP_CONVERTER_IMAGETRANSFORMCONTROLLER_H
