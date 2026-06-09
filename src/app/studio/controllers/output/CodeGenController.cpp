#include "app/studio/controllers/output/CodeGenController.h"
#include "app/studio/DisplayConverter.h"
#include "app/studio/model/ConverterState.h"
#include "app/studio/pipeline/ImagePipelineController.h"
#include "processing/DisplayCodeGenerator.h"

CodeGenController::CodeGenController(QObject *parent)
    : QObject(parent)
{
}

void CodeGenController::attach(DisplayConverter *host, ConverterState *state)
{
    m_host = host;
    m_state = state;
}

QString CodeGenController::arrayName() const
{
    return m_state ? m_state->arrayName : QString();
}

bool CodeGenController::codeIncludeComments() const
{
    return m_state && m_state->codeGenOptions.includeHeaderComments;
}

bool CodeGenController::codeUseProgmem() const
{
    return m_state && m_state->codeGenOptions.useProgmem;
}

bool CodeGenController::codeStaticStorage() const
{
    return m_state && m_state->codeGenOptions.staticStorage;
}

bool CodeGenController::rgb565BigEndian() const
{
    return m_state && m_state->codeGenOptions.rgb565BigEndian;
}

int CodeGenController::codeDmaAlign() const
{
    return m_state ? m_state->codeGenOptions.dmaPaddingAlign : 0;
}

bool CodeGenController::showFullGeneratedCode() const
{
    return m_state && m_state->showFullGeneratedCode;
}

void CodeGenController::requestRebuild(bool immediate)
{
    if (m_host)
        ImagePipelineController::scheduleRebuild(*m_host, immediate);
}

void CodeGenController::setArrayName(const QString &name)
{
    if (!m_state)
        return;
    const QString safe = DisplayCodeGenerator::sanitizeIdentifier(name);
    if (m_state->arrayName == safe)
        return;
    m_state->arrayName = safe;
    emit arrayNameChanged();
    if (m_host) {
        m_host->schedulePersistSession();
        if (m_host->hasImage())
            requestRebuild();
    }
}

void CodeGenController::setCodeIncludeComments(bool on)
{
    if (!m_state || m_state->codeGenOptions.includeHeaderComments == on)
        return;
    m_state->codeGenOptions.includeHeaderComments = on;
    emit codeGenOptionsChanged();
    requestRebuild(true);
}

void CodeGenController::setCodeUseProgmem(bool on)
{
    if (!m_state || m_state->codeGenOptions.useProgmem == on)
        return;
    m_state->codeGenOptions.useProgmem = on;
    emit codeGenOptionsChanged();
    requestRebuild(true);
}

void CodeGenController::setCodeStaticStorage(bool on)
{
    if (!m_state || m_state->codeGenOptions.staticStorage == on)
        return;
    m_state->codeGenOptions.staticStorage = on;
    emit codeGenOptionsChanged();
    requestRebuild(true);
}

void CodeGenController::setRgb565BigEndian(bool on)
{
    if (!m_state || m_state->codeGenOptions.rgb565BigEndian == on)
        return;
    m_state->codeGenOptions.rgb565BigEndian = on;
    emit rgb565BigEndianChanged();
    emit codeGenOptionsChanged();
    requestRebuild(true);
}

void CodeGenController::setCodeDmaAlign(int align)
{
    if (!m_state)
        return;
    align = align == 8 ? 8 : (align == 4 ? 4 : 0);
    if (m_state->codeGenOptions.dmaPaddingAlign == align)
        return;
    m_state->codeGenOptions.dmaPaddingAlign = align;
    emit codeDmaAlignChanged();
    emit codeGenOptionsChanged();
    requestRebuild(true);
}

void CodeGenController::setShowFullGeneratedCode(bool on)
{
    if (!m_state || m_state->showFullGeneratedCode == on)
        return;
    m_state->showFullGeneratedCode = on;
    emit showFullGeneratedCodeChanged();
    if (m_host)
        ImagePipelineController::updateCodePreview(*m_state, *m_host);
}

void CodeGenController::copyGeneratedArray()
{
    if (!m_host || !m_state)
        return;
    m_host->exportPanel()->copyToClipboard(DisplayCodeGenerator::extractArrayBody(m_state->generatedCode));
}

void CodeGenController::notifyAllChanged()
{
    emit arrayNameChanged();
    emit codeGenOptionsChanged();
    emit rgb565BigEndianChanged();
    emit codeDmaAlignChanged();
    emit showFullGeneratedCodeChanged();
}
