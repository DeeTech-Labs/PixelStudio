#ifndef PIXELSTUDIO_APP_PANELS_INSPECTORIMAGEPRESENTER_H
#define PIXELSTUDIO_APP_PANELS_INSPECTORIMAGEPRESENTER_H

#include <QObject>
#include <QVariantList>

class DisplayConverter;
class ImageFilterController;
class ImageTransformController;
class DisplayOutputController;

class InspectorImagePresenter : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool hasImage READ hasImage NOTIFY hasImageChanged)
    Q_PROPERTY(bool hasPreview READ hasPreview NOTIFY hasPreviewChanged)
    Q_PROPERTY(int sourceWidth READ sourceWidth NOTIFY sourceWidthChanged)
    Q_PROPERTY(int sourceHeight READ sourceHeight NOTIFY sourceHeightChanged)
    Q_PROPERTY(int previewColorCount READ previewColorCount NOTIFY previewColorCountChanged)
    Q_PROPERTY(QString imageFormatName READ imageFormatName NOTIFY imageFormatNameChanged)
    Q_PROPERTY(bool monoOutput READ monoOutput NOTIFY monoOutputChanged)
    Q_PROPERTY(bool grayscaleOutput READ grayscaleOutput NOTIFY grayscaleOutputChanged)
    Q_PROPERTY(bool toneLocked READ toneLocked NOTIFY toneLockedChanged)
    Q_PROPERTY(QVariantList encodingModesModel READ encodingModesModel NOTIFY encodingModesModelChanged)
    Q_PROPERTY(QObject *filters READ filters CONSTANT)
    Q_PROPERTY(QObject *transform READ transform CONSTANT)
    Q_PROPERTY(QObject *output READ output CONSTANT)

public:
    explicit InspectorImagePresenter(DisplayConverter *converter, QObject *parent = nullptr);

    bool hasImage() const;
    bool hasPreview() const;
    int sourceWidth() const;
    int sourceHeight() const;
    int previewColorCount() const;
    QString imageFormatName() const;
    bool monoOutput() const;
    bool grayscaleOutput() const;
    bool toneLocked() const;
    QVariantList encodingModesModel() const;

    QObject *filters() const;
    QObject *transform() const;
    QObject *output() const;
    Q_INVOKABLE void resetFilters();

signals:
    void hasImageChanged();
    void hasPreviewChanged();
    void sourceWidthChanged();
    void sourceHeightChanged();
    void previewColorCountChanged();
    void imageFormatNameChanged();
    void monoOutputChanged();
    void grayscaleOutputChanged();
    void toneLockedChanged();
    void encodingModesModelChanged();

private:
    void connectSignals();

    DisplayConverter *m_converter = nullptr;
    ImageFilterController *m_filters = nullptr;
    ImageTransformController *m_transform = nullptr;
    DisplayOutputController *m_output = nullptr;
};

#endif // PIXELSTUDIO_APP_PANELS_INSPECTORIMAGEPRESENTER_H
