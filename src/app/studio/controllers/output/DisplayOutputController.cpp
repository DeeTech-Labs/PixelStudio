#include "app/studio/controllers/output/DisplayOutputController.h"
#include "app/studio/DisplayConverter.h"
#include "app/studio/model/ConverterEncoding.h"
#include "app/studio/model/ConverterState.h"
#include "app/studio/controllers/image/ImageFilterController.h"
#include "app/studio/pipeline/ImagePipelineController.h"
#include "processing/DisplayCodeGenerator.h"
#include "processing/DisplayProfile.h"
#include "processing/PixelFormatCatalog.h"
#include "translation/AppLocale.h"

DisplayOutputController::DisplayOutputController(QObject *parent)
    : QObject(parent)
{
}

void DisplayOutputController::attach(DisplayConverter *host, ConverterState *state)
{
    m_host = host;
    m_state = state;
}

int DisplayOutputController::displayWidth() const
{
    return m_state ? m_state->displayWidth : 0;
}

int DisplayOutputController::displayHeight() const
{
    return m_state ? m_state->displayHeight : 0;
}

QString DisplayOutputController::profileId() const
{
    return m_state ? m_state->profileId : QString();
}

int DisplayOutputController::colorMode() const
{
    return m_state ? static_cast<int>(m_state->colorMode) : 0;
}

QString DisplayOutputController::colorModeName() const
{
    if (!m_state)
        return {};
    return m_state->colorMode == DisplayProfile::Rgb565
        ? AppLocale::tr("Color")
        : AppLocale::tr("B&W");
}

int DisplayOutputController::encodingMode() const
{
    return m_state ? static_cast<int>(m_state->encodingMode) : 0;
}

QString DisplayOutputController::encodingModeName() const
{
    const QVariantList modes = availableEncodingModes();
    for (const QVariant &v : modes) {
        const QVariantMap m = v.toMap();
        if (m.value(QStringLiteral("mode")).toInt() == encodingMode())
            return m.value(QStringLiteral("name")).toString();
    }
    return AppLocale::tr("Unknown");
}

bool DisplayOutputController::encodingIsMono1Bit() const
{
    return m_state && ConverterEncoding::isMonoMode(encodingMode());
}

bool DisplayOutputController::encodingIsGrayscale() const
{
    return m_state && PixelFormatCatalog::isGrayscale(m_state->encodingMode);
}

bool DisplayOutputController::encodingIsColor() const
{
    return m_state && ConverterEncoding::isColorMode(encodingMode());
}

int DisplayOutputController::monoLayout() const
{
    return m_state ? static_cast<int>(m_state->monoLayout) : 0;
}

QString DisplayOutputController::monoLayoutName() const
{
    using Layout = DisplayCodeGenerator::MonoLayout;
    if (!m_state)
        return {};
    if (m_state->monoLayout == Layout::Ssd1306Page)
        return AppLocale::tr("Vertical page buffer");
    if (m_state->monoLayout == Layout::VerticalColumn)
        return AppLocale::tr("Vertical column");
    return AppLocale::tr("Row-packed");
}

int DisplayOutputController::monoThreshold() const
{
    return m_state ? m_state->monoThreshold : 128;
}

int DisplayOutputController::dataByteCount() const
{
    if (!m_state)
        return 0;
    return DisplayCodeGenerator::flashFootprintBytes(m_state->encodingMode,
                                                     m_state->displayWidth,
                                                     m_state->displayHeight,
                                                     m_state->lastResult.monoBits,
                                                     m_state->lastResult.monoBuffer,
                                                     m_state->lastResult.grayscale8,
                                                     m_state->lastResult.rgb565,
                                                     m_state->lastResult.rgb888,
                                                     m_state->lastResult.rgb233,
                                                     m_state->lastResult.rgb24,
                                                     m_state->monoLayout,
                                                     m_state->codeGenOptions,
                                                     m_state->lastResult.indexedPalette);
}

bool DisplayOutputController::linearColorSpace() const
{
    return !m_state || m_state->linearColorSpace;
}

void DisplayOutputController::requestRebuild(bool immediate)
{
    if (m_host)
        ImagePipelineController::scheduleRebuild(*m_host, immediate);
}

void DisplayOutputController::applyProfile(const DisplayProfile &profile)
{
    if (!m_state || profile.id == QStringLiteral("custom"))
        return;

    m_state->displayWidth = profile.width;
    m_state->displayHeight = profile.height;
    emit displayWidthChanged();
    emit displayHeightChanged();
}

void DisplayOutputController::syncProfileFromDimensions()
{
    if (!m_state)
        return;

    for (const DisplayProfile &p : DisplayProfile::presets()) {
        if (p.id == QStringLiteral("custom"))
            continue;
        if (p.width == m_state->displayWidth && p.height == m_state->displayHeight) {
            if (m_state->profileId != p.id) {
                m_state->profileId = p.id;
                emit profileIdChanged();
            }
            return;
        }
    }
    if (m_state->profileId != QStringLiteral("custom")) {
        m_state->profileId = QStringLiteral("custom");
        emit profileIdChanged();
    }
}

void DisplayOutputController::setDisplayWidth(int w)
{
    if (!m_state)
        return;
    w = qBound(8, w, 1024);
    if (m_state->displayWidth == w)
        return;
    m_state->displayWidth = w;
    emit displayWidthChanged();
    emit generatedFootprintChanged();
    syncProfileFromDimensions();
    requestRebuild();
}

void DisplayOutputController::setDisplayHeight(int h)
{
    if (!m_state)
        return;
    h = qBound(8, h, 1024);
    if (m_state->displayHeight == h)
        return;
    m_state->displayHeight = h;
    emit displayHeightChanged();
    emit generatedFootprintChanged();
    syncProfileFromDimensions();
    requestRebuild();
}

void DisplayOutputController::setProfileId(const QString &id)
{
    if (!m_state || m_state->profileId == id)
        return;
    m_state->profileId = id;
    if (id != QStringLiteral("custom"))
        applyProfile(DisplayProfile::byId(id));
    emit profileIdChanged();
    emit displayWidthChanged();
    emit displayHeightChanged();
    emit generatedFootprintChanged();
    requestRebuild();
}

void DisplayOutputController::setColorMode(int mode)
{
    if (!m_state)
        return;
    const auto next = mode == static_cast<int>(DisplayProfile::Rgb565)
                          ? DisplayProfile::Rgb565
                          : DisplayProfile::Mono1Bit;
    if (m_state->colorMode == next)
        return;
    m_state->colorMode = next;

    const int enc = static_cast<int>(m_state->encodingMode);
    if (m_state->colorMode == DisplayProfile::Rgb565) {
        if (!ConverterEncoding::isColorMode(enc))
            m_state->encodingMode = DisplayCodeGenerator::EncodingMode::Rgb565;
    } else {
        if (!ConverterEncoding::isMonoMode(enc))
            m_state->encodingMode = DisplayCodeGenerator::EncodingMode::Mono1Bit;
    }

    if (m_state->colorMode != DisplayProfile::Mono1Bit
        && m_state->monoLayout != DisplayCodeGenerator::MonoLayout::RowPacked) {
        m_state->monoLayout = DisplayCodeGenerator::MonoLayout::RowPacked;
        emit monoLayoutChanged();
    }

    emit encodingModeChanged();
    emit generatedFootprintChanged();
    emit colorModeChanged();
    syncProfileFromDimensions();
    requestRebuild();
}

void DisplayOutputController::setEncodingMode(int mode)
{
    if (!m_state)
        return;
    const auto next = static_cast<DisplayCodeGenerator::EncodingMode>(
        qBound(0, mode, static_cast<int>(DisplayCodeGenerator::EncodingMode::Count) - 1));
    if (m_state->encodingMode == next)
        return;
    m_state->encodingMode = next;

    const auto nextColor = ConverterEncoding::isColorMode(static_cast<int>(next))
                               ? DisplayProfile::Rgb565
                               : DisplayProfile::Mono1Bit;
    if (m_state->colorMode != nextColor) {
        m_state->colorMode = nextColor;
        emit colorModeChanged();
    }
    if (m_state->colorMode != DisplayProfile::Mono1Bit
        && m_state->monoLayout != DisplayCodeGenerator::MonoLayout::RowPacked) {
        m_state->monoLayout = DisplayCodeGenerator::MonoLayout::RowPacked;
        emit monoLayoutChanged();
    }

    emit encodingModeChanged();
    emit generatedFootprintChanged();
    if (m_host && m_host->hasImage())
        requestRebuild();
}

void DisplayOutputController::setMonoLayout(int mode)
{
    if (!m_state)
        return;
    const auto next = static_cast<DisplayCodeGenerator::MonoLayout>(qBound(0, mode, 2));
    if (m_state->monoLayout == next)
        return;
    m_state->monoLayout = next;
    emit monoLayoutChanged();
    emit generatedFootprintChanged();
    if (m_host && m_host->hasImage() && encodingIsMono1Bit())
        requestRebuild();
}

void DisplayOutputController::setMonoThreshold(int value)
{
    if (!m_state)
        return;
    value = qBound(0, value, 255);
    if (m_state->monoThreshold == value)
        return;
    m_state->monoThreshold = value;
    if (m_host)
        m_host->imageFilters()->syncThreshold(value);
    emit monoThresholdChanged();
    if (m_host && m_host->hasImage() && encodingIsMono1Bit())
        requestRebuild();
}

void DisplayOutputController::setLinearColorSpace(bool on)
{
    if (!m_state || m_state->linearColorSpace == on)
        return;
    m_state->linearColorSpace = on;
    emit linearColorSpaceChanged();
    if (m_host && m_host->hasImage())
        requestRebuild();
}

void DisplayOutputController::swapDisplayDimensions()
{
    if (!m_state)
        return;
    const int w = m_state->displayWidth;
    const int h = m_state->displayHeight;
    if (w == h)
        return;
    m_state->displayWidth = h;
    m_state->displayHeight = w;
    emit displayWidthChanged();
    emit displayHeightChanged();
    emit generatedFootprintChanged();
    syncProfileFromDimensions();
    requestRebuild();
}

QVariantList DisplayOutputController::displayPresets() const
{
    QVariantList list;
    for (const DisplayProfile &p : DisplayProfile::presets()) {
        QVariantMap m;
        m[QStringLiteral("id")] = p.id;
        m[QStringLiteral("name")] = AppLocale::tr(p.name.toUtf8().constData());
        m[QStringLiteral("width")] = p.width;
        m[QStringLiteral("height")] = p.height;
        list.append(m);
    }
    return list;
}

QVariantList DisplayOutputController::availableEncodingModes() const
{
    QVariantList list = DisplayCodeGenerator::availableEncodings();
    for (QVariant &item : list) {
        QVariantMap m = item.toMap();
        const QString name = m.value(QStringLiteral("name")).toString();
        if (!name.isEmpty())
            m.insert(QStringLiteral("name"), AppLocale::tr(name.toUtf8().constData()));
        item = m;
    }
    return list;
}

QVariantList DisplayOutputController::availableEncodingModesForUi() const
{
    return availableEncodingModes();
}

void DisplayOutputController::notifyAllChanged()
{
    emit displayWidthChanged();
    emit displayHeightChanged();
    emit profileIdChanged();
    emit colorModeChanged();
    emit encodingModeChanged();
    emit monoLayoutChanged();
    emit monoThresholdChanged();
    emit linearColorSpaceChanged();
    emit generatedFootprintChanged();
}

void DisplayOutputController::notifyFootprintChanged()
{
    emit generatedFootprintChanged();
}
