#ifndef PIXELSTUDIO_APP_PANELS_INSPECTORDISPLAYPRESENTER_H
#define PIXELSTUDIO_APP_PANELS_INSPECTORDISPLAYPRESENTER_H

#include <QObject>

class DisplayConverter;
class DisplayOutputController;
class CodeGenController;

class InspectorDisplayPresenter : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool monoEncoding READ monoEncoding NOTIFY monoEncodingChanged)
    Q_PROPERTY(QObject *output READ output CONSTANT)
    Q_PROPERTY(QObject *code READ code CONSTANT)
    Q_PROPERTY(QObject *image READ image CONSTANT)

public:
    explicit InspectorDisplayPresenter(DisplayConverter *converter, QObject *parent = nullptr);

    bool monoEncoding() const;

    QObject *output() const;
    QObject *code() const;
    QObject *image() const;

    Q_INVOKABLE void applySmallestEncoding();

signals:
    void monoEncodingChanged();

private:
    void connectSignals();

    DisplayConverter *m_converter = nullptr;
    DisplayOutputController *m_output = nullptr;
    CodeGenController *m_code = nullptr;
};

#endif // PIXELSTUDIO_APP_PANELS_INSPECTORDISPLAYPRESENTER_H
