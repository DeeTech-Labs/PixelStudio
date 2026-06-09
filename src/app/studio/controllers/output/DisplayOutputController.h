#ifndef PIXELSTUDIO_APP_CONVERTER_DISPLAYOUTPUTCONTROLLER_H
#define PIXELSTUDIO_APP_CONVERTER_DISPLAYOUTPUTCONTROLLER_H

#include <QObject>
#include <QVariantList>

#include "processing/DisplayProfile.h"

class DisplayConverter;
struct ConverterState;

class DisplayOutputController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int displayWidth READ displayWidth WRITE setDisplayWidth NOTIFY displayWidthChanged)
    Q_PROPERTY(int displayHeight READ displayHeight WRITE setDisplayHeight NOTIFY displayHeightChanged)
    Q_PROPERTY(QString profileId READ profileId WRITE setProfileId NOTIFY profileIdChanged)
    Q_PROPERTY(int colorMode READ colorMode WRITE setColorMode NOTIFY colorModeChanged)
    Q_PROPERTY(QString colorModeName READ colorModeName NOTIFY colorModeChanged)
    Q_PROPERTY(int encodingMode READ encodingMode WRITE setEncodingMode NOTIFY encodingModeChanged)
    Q_PROPERTY(QString encodingModeName READ encodingModeName NOTIFY encodingModeChanged)
    Q_PROPERTY(bool encodingIsMono1Bit READ encodingIsMono1Bit NOTIFY encodingModeChanged)
    Q_PROPERTY(bool encodingIsGrayscale READ encodingIsGrayscale NOTIFY encodingModeChanged)
    Q_PROPERTY(bool encodingIsColor READ encodingIsColor NOTIFY encodingModeChanged)
    Q_PROPERTY(int monoLayout READ monoLayout WRITE setMonoLayout NOTIFY monoLayoutChanged)
    Q_PROPERTY(QString monoLayoutName READ monoLayoutName NOTIFY monoLayoutChanged)
    Q_PROPERTY(int monoThreshold READ monoThreshold WRITE setMonoThreshold NOTIFY monoThresholdChanged)
    Q_PROPERTY(int dataByteCount READ dataByteCount NOTIFY generatedFootprintChanged)
    Q_PROPERTY(bool linearColorSpace READ linearColorSpace WRITE setLinearColorSpace NOTIFY linearColorSpaceChanged)
    Q_PROPERTY(QVariantList displayPresetsModel READ displayPresetsModel NOTIFY displayPresetsModelChanged)
    Q_PROPERTY(QVariantList encodingModesModel READ encodingModesModel NOTIFY encodingModesModelChanged)

public:
    explicit DisplayOutputController(QObject *parent = nullptr);

    void attach(DisplayConverter *host, ConverterState *state);

    int displayWidth() const;
    int displayHeight() const;
    QString profileId() const;
    int colorMode() const;
    QString colorModeName() const;
    int encodingMode() const;
    QString encodingModeName() const;
    bool encodingIsMono1Bit() const;
    bool encodingIsGrayscale() const;
    bool encodingIsColor() const;
    int monoLayout() const;
    QString monoLayoutName() const;
    int monoThreshold() const;
    int dataByteCount() const;
    bool linearColorSpace() const;
    QVariantList displayPresetsModel() const { return m_displayPresetsModel; }
    QVariantList encodingModesModel() const { return m_encodingModesModel; }

    Q_INVOKABLE void setDisplayWidth(int w);
    Q_INVOKABLE void setDisplayHeight(int h);
    Q_INVOKABLE void setProfileId(const QString &id);
    Q_INVOKABLE void setColorMode(int mode);
    Q_INVOKABLE void setEncodingMode(int mode);
    Q_INVOKABLE void setMonoLayout(int mode);
    Q_INVOKABLE void setMonoThreshold(int value);
    Q_INVOKABLE void setLinearColorSpace(bool on);
    Q_INVOKABLE void swapDisplayDimensions();
    Q_INVOKABLE QVariantList displayPresets() const;
    Q_INVOKABLE QVariantList availableEncodingModes() const;
    Q_INVOKABLE QVariantList availableEncodingModesForUi() const;

    void notifyAllChanged();
    void notifyFootprintChanged();
    void syncProfileFromDimensions();

signals:
    void displayWidthChanged();
    void displayHeightChanged();
    void profileIdChanged();
    void colorModeChanged();
    void encodingModeChanged();
    void monoLayoutChanged();
    void monoThresholdChanged();
    void linearColorSpaceChanged();
    void generatedFootprintChanged();
    void displayPresetsModelChanged();
    void encodingModesModelChanged();

private:
    void applyProfile(const DisplayProfile &profile);
    void requestRebuild(bool immediate = false);
    void rebuildUiModels();

    DisplayConverter *m_host = nullptr;
    ConverterState *m_state = nullptr;
    QVariantList m_displayPresetsModel;
    QVariantList m_encodingModesModel;
};

#endif // PIXELSTUDIO_APP_CONVERTER_DISPLAYOUTPUTCONTROLLER_H
