#include "app/panels/InspectorImagePresenter.h"

#include "app/studio/DisplayConverter.h"
#include "app/studio/controllers/image/ImageFilterController.h"
#include "app/studio/controllers/image/ImageTransformController.h"
#include "app/studio/controllers/output/DisplayOutputController.h"

InspectorImagePresenter::InspectorImagePresenter(DisplayConverter *converter, QObject *parent)
    : QObject(parent)
    , m_converter(converter)
{
    if (!m_converter)
        return;
    m_filters = m_converter->imageFilters();
    m_transform = m_converter->imageTransform();
    m_output = m_converter->displayOutput();
    connectSignals();
}

bool InspectorImagePresenter::hasImage() const
{
    return m_converter && m_converter->hasImage();
}

bool InspectorImagePresenter::hasPreview() const
{
    return m_converter && m_converter->hasPreview();
}

int InspectorImagePresenter::sourceWidth() const
{
    return m_converter ? m_converter->sourceWidth() : 0;
}

int InspectorImagePresenter::sourceHeight() const
{
    return m_converter ? m_converter->sourceHeight() : 0;
}

int InspectorImagePresenter::previewColorCount() const
{
    return m_converter ? m_converter->previewColorCount() : 0;
}

QString InspectorImagePresenter::imageFormatName() const
{
    return m_converter ? m_converter->imageFormatName() : QString();
}

bool InspectorImagePresenter::monoOutput() const
{
    return m_output && m_output->encodingIsMono1Bit();
}

bool InspectorImagePresenter::grayscaleOutput() const
{
    return m_output && m_output->encodingIsGrayscale();
}

bool InspectorImagePresenter::toneLocked() const
{
    return m_filters && m_filters->tonePreset() != 0;
}

QVariantList InspectorImagePresenter::encodingModesModel() const
{
    return m_output ? m_output->encodingModesModel() : QVariantList{};
}

QObject *InspectorImagePresenter::filters() const
{
    return m_filters;
}

QObject *InspectorImagePresenter::transform() const
{
    return m_transform;
}

QObject *InspectorImagePresenter::output() const
{
    return m_output;
}

void InspectorImagePresenter::resetFilters()
{
    if (m_filters)
        m_filters->resetFilters();
}

void InspectorImagePresenter::connectSignals()
{
    if (!m_converter)
        return;

    connect(m_converter, &DisplayConverter::hasImageChanged, this, &InspectorImagePresenter::hasImageChanged);
    connect(m_converter, &DisplayConverter::hasImageChanged, this, &InspectorImagePresenter::imageFormatNameChanged);
    connect(m_converter, &DisplayConverter::sourceWidthChanged, this, &InspectorImagePresenter::sourceWidthChanged);
    connect(m_converter, &DisplayConverter::sourceHeightChanged, this, &InspectorImagePresenter::sourceHeightChanged);
    connect(m_converter, &DisplayConverter::previewPathChanged, this, &InspectorImagePresenter::hasPreviewChanged);
    connect(m_converter, &DisplayConverter::previewPathChanged, this, &InspectorImagePresenter::previewColorCountChanged);

    if (m_output) {
        connect(m_output, &DisplayOutputController::encodingModeChanged, this, &InspectorImagePresenter::monoOutputChanged);
        connect(m_output, &DisplayOutputController::encodingModeChanged, this, &InspectorImagePresenter::grayscaleOutputChanged);
        connect(m_output, &DisplayOutputController::encodingModesModelChanged, this,
                &InspectorImagePresenter::encodingModesModelChanged);
    }

    if (m_filters) {
        connect(m_filters, &ImageFilterController::tonePresetChanged, this, &InspectorImagePresenter::toneLockedChanged);
    }
}
