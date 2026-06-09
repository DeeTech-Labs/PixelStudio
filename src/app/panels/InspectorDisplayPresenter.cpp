#include "app/panels/InspectorDisplayPresenter.h"

#include "app/studio/DisplayConverter.h"
#include "app/studio/controllers/output/CodeGenController.h"
#include "app/studio/controllers/output/DisplayOutputController.h"

InspectorDisplayPresenter::InspectorDisplayPresenter(DisplayConverter *converter, QObject *parent)
    : QObject(parent)
    , m_converter(converter)
{
    if (!m_converter)
        return;
    m_output = m_converter->displayOutput();
    m_code = m_converter->codeGen();
    connectSignals();
}

bool InspectorDisplayPresenter::monoEncoding() const
{
    return m_output && m_output->encodingIsMono1Bit();
}

QObject *InspectorDisplayPresenter::output() const
{
    return m_output;
}

QObject *InspectorDisplayPresenter::code() const
{
    return m_code;
}

QObject *InspectorDisplayPresenter::image() const
{
    return m_converter;
}

void InspectorDisplayPresenter::applySmallestEncoding()
{
    if (!m_converter || !m_output)
        return;

    const QVariantList report = m_converter->flashReport();
    for (const QVariant &rowVar : report) {
        const QVariantMap row = rowVar.toMap();
        if (row.value(QStringLiteral("recommended")).toBool()) {
            const int mode = row.value(QStringLiteral("mode")).toInt();
            m_output->setEncodingMode(mode);
            return;
        }
    }
}

void InspectorDisplayPresenter::connectSignals()
{
    if (!m_output)
        return;

    connect(m_output, &DisplayOutputController::encodingModeChanged, this,
            &InspectorDisplayPresenter::monoEncodingChanged);
}
