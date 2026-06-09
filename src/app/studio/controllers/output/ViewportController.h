#ifndef PIXELSTUDIO_APP_CONVERTER_VIEWPORTCONTROLLER_H
#define PIXELSTUDIO_APP_CONVERTER_VIEWPORTCONTROLLER_H

#include <QObject>

class DisplayConverter;
struct ConverterState;

class ViewportController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool showGrid READ showGrid WRITE setShowGrid NOTIFY showGridChanged)
    Q_PROPERTY(int gridThresholdZoom READ gridThresholdZoom WRITE setGridThresholdZoom NOTIFY gridThresholdZoomChanged)

public:
    explicit ViewportController(QObject *parent = nullptr);

    void attach(DisplayConverter *host, ConverterState *state);

    bool showGrid() const;
    int gridThresholdZoom() const;

    Q_INVOKABLE void setShowGrid(bool on);
    Q_INVOKABLE void setGridThresholdZoom(int value);

    void notifyAllChanged();

signals:
    void showGridChanged();
    void gridThresholdZoomChanged();

private:
    DisplayConverter *m_host = nullptr;
    ConverterState *m_state = nullptr;
};

#endif // PIXELSTUDIO_APP_CONVERTER_VIEWPORTCONTROLLER_H
