#include "app/studio/controllers/output/ViewportController.h"
#include "app/studio/DisplayConverter.h"
#include "app/studio/model/ConverterState.h"

ViewportController::ViewportController(QObject *parent)
    : QObject(parent)
{
}

void ViewportController::attach(DisplayConverter *host, ConverterState *state)
{
    m_host = host;
    m_state = state;
}

bool ViewportController::showGrid() const
{
    return m_state && m_state->showGrid;
}

int ViewportController::gridThresholdZoom() const
{
    return m_state ? m_state->gridThresholdZoom : 8;
}

void ViewportController::setShowGrid(bool on)
{
    if (!m_state || m_state->showGrid == on)
        return;
    m_state->showGrid = on;
    emit showGridChanged();
    if (m_host)
        m_host->schedulePersistSession();
}

void ViewportController::setGridThresholdZoom(int value)
{
    if (!m_state)
        return;
    value = qBound(1, value, 64);
    if (m_state->gridThresholdZoom == value)
        return;
    m_state->gridThresholdZoom = value;
    emit gridThresholdZoomChanged();
    if (m_host)
        m_host->schedulePersistSession();
}

void ViewportController::notifyAllChanged()
{
    emit showGridChanged();
    emit gridThresholdZoomChanged();
}
