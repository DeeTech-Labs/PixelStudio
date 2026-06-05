#include "app/DisplayConverter.h"
#include "i18n/AppLocale.h"
#include "persistence/AppPaths.h"
#include "persistence/AppSettings.h"
#include "processing/DisplayCodeGenerator.h"
#include "processing/PixelFormatCatalog.h"
#include "processing/ControllerCatalog.h"
#include "processing/EncodingAnalyzer.h"
#include "io/ImageLoader.h"
#include "io/BinaryExporter.h"
#include "io/HeaderImportService.h"
#include "persistence/SessionSettings.h"
#include "persistence/ProjectService.h"
#include "processing/ConvertPipeline.h"
#include "export/BatchExportService.h"
#include "export/SpriteAtlasService.h"

#include <QCoreApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QImage>
#include <QSet>
#include <QSize>
#include <QtConcurrent>
#include <QGuiApplication>
#include <QTextStream>
#include <QBuffer>
#include <QFileInfo>
#include <QSaveFile>
#include <QTransform>
#include <QVariantMap>

namespace {

bool encodingIsColorMode(int mode)
{
    return !PixelFormatCatalog::isMono(static_cast<DisplayCodeGenerator::EncodingMode>(mode));
}

bool encodingIsMonoMode(int mode)
{
    return PixelFormatCatalog::isMono(static_cast<DisplayCodeGenerator::EncodingMode>(mode));
}

QString rgbToHex(int r, int g, int b)
{
    return QStringLiteral("#%1%2%3")
        .arg(r, 2, 16, QChar('0'))
        .arg(g, 2, 16, QChar('0'))
        .arg(b, 2, 16, QChar('0'))
        .toUpper();
}

QVariantList paletteFromRgbList(const QVector<quint32> &colors)
{
    QVariantList out;
    out.reserve(colors.size());
    for (quint32 c : colors) {
        out.append(rgbToHex(int((c >> 16) & 0xFF), int((c >> 8) & 0xFF), int(c & 0xFF)));
    }
    return out;
}

QVariantList paletteFromPreviewImage(const QImage &img, int maxColors = 256)
{
    QVariantList out;
    if (img.isNull() || maxColors < 1)
        return out;

    const QImage sample = (img.width() > 160 || img.height() > 160)
        ? img.scaled(160, 160, Qt::KeepAspectRatio, Qt::FastTransformation)
        : img;

    QSet<quint32> seen;
    QVector<quint32> ordered;
    ordered.reserve(qMin(maxColors, sample.width() * sample.height()));

    for (int y = 0; y < sample.height(); ++y) {
        for (int x = 0; x < sample.width(); ++x) {
            const QRgb px = sample.pixel(x, y);
            const int a = qAlpha(px);
            if (a < 8)
                continue;
            const quint32 key = quint32(0xFF000000)
                | quint32(qRed(px) << 16)
                | quint32(qGreen(px) << 8)
                | quint32(qBlue(px));
            if (seen.contains(key))
                continue;
            seen.insert(key);
            ordered.append(key);
            if (ordered.size() >= maxColors)
                break;
        }
        if (ordered.size() >= maxColors)
            break;
    }

    return paletteFromRgbList(ordered);
}

} // namespace

DisplayConverter::DisplayConverter(SessionSettings *session, AppSettings *appSettings, QObject *parent)
    : QObject(parent)
    , m_loader(new ImageLoader(this))
    , m_session(session)
    , m_appSettings(appSettings)
{
    connect(m_loader, &ImageLoader::loaded, this, &DisplayConverter::onImageLoaded);
    connect(m_loader, &ImageLoader::error, this, &DisplayConverter::errorOccurred);
    m_filterParams.ditherMode = ImageFiltersPipeline::DitherMode::FloydSteinberg;
    m_filterParams.threshold = m_monoThreshold;
    m_rebuildDebounceTimer.setSingleShot(true);
    m_rebuildDebounceTimer.setInterval(200);
    connect(&m_rebuildDebounceTimer, &QTimer::timeout, this, &DisplayConverter::startAsyncRebuild);
    connect(&m_rebuildWatcher, &QFutureWatcher<AsyncBuildResult>::finished,
            this, &DisplayConverter::onAsyncRebuildFinished);
    m_sessionSaveTimer.setSingleShot(true);
    m_sessionSaveTimer.setInterval(400);
    connect(&m_sessionSaveTimer, &QTimer::timeout, this, &DisplayConverter::persistSession);
    m_autosaveTimer.setSingleShot(false);
    connect(&m_autosaveTimer, &QTimer::timeout, this, &DisplayConverter::onAutosaveTimeout);
    connect(&m_batchService, &BatchExportService::runningChanged, this, &DisplayConverter::batchRunningChanged);
    connect(&m_batchService, &BatchExportService::progressChanged, this, &DisplayConverter::batchProgressChanged);
    connect(&m_batchService, &BatchExportService::finished, this, &DisplayConverter::onBatchFinished);
    connect(&m_watchService, &WatchFolderService::activeChanged, this, &DisplayConverter::watchFolderChanged);
    connect(&m_watchService, &WatchFolderService::foldersChanged, this, &DisplayConverter::watchFolderChanged);
    connect(&m_watchService, &WatchFolderService::filesChanged, this, &DisplayConverter::watchFolderChanged);
    connect(&m_watchService, &WatchFolderService::exportRequested, this, &DisplayConverter::onWatchExportRequested);
    if (m_appSettings) {
        connect(m_appSettings, &AppSettings::projectAutosaveChanged, this, &DisplayConverter::restartAutosaveTimer);
        connect(m_appSettings, &AppSettings::projectAutosaveSecondsChanged, this, &DisplayConverter::restartAutosaveTimer);
    }
    if (m_session) {
        connect(m_session, &SessionSettings::recentFilesChanged, this, &DisplayConverter::recentFilesChanged);
        connect(m_session, &SessionSettings::recentExportsChanged, this, &DisplayConverter::recentExportsChanged);
        applySessionSnapshot(SessionSettings::defaultSnapshot());
        m_session->loadUiState(&m_uiState);
        m_session->pruneMissingRecentFiles();
        m_session->pruneMissingRecentExports();
        applyStoredUiState();
        updateWatchExportPrefix();
        restartAutosaveTimer();
    } else {
        applyProfile(DisplayProfile::byId(m_profileId));
    }
}

void DisplayConverter::setDisplayWidth(int w)
{
    w = qBound(8, w, 1024);
    if (m_displayWidth == w)
        return;
    m_displayWidth = w;
    emit displayWidthChanged();
    syncProfileIdFromDimensions();
    scheduleRebuild();
}

void DisplayConverter::setDisplayHeight(int h)
{
    h = qBound(8, h, 1024);
    if (m_displayHeight == h)
        return;
    m_displayHeight = h;
    emit displayHeightChanged();
    syncProfileIdFromDimensions();
    scheduleRebuild();
}

void DisplayConverter::setProfileId(const QString &id)
{
    if (m_profileId == id)
        return;
    m_profileId = id;
    if (id != QStringLiteral("custom"))
        applyProfile(DisplayProfile::byId(id));
    emit profileIdChanged();
    emit displayWidthChanged();
    emit displayHeightChanged();
    scheduleRebuild();
}

void DisplayConverter::setColorMode(int mode)
{
    const auto next = mode == static_cast<int>(DisplayProfile::Rgb565)
                          ? DisplayProfile::Rgb565
                          : DisplayProfile::Mono1Bit;
    if (m_colorMode == next)
        return;
    m_colorMode = next;

    const int enc = static_cast<int>(m_encodingMode);
    if (m_colorMode == DisplayProfile::Rgb565) {
        if (!encodingIsColorMode(enc))
            m_encodingMode = DisplayCodeGenerator::EncodingMode::Rgb565;
    } else {
        if (!encodingIsMonoMode(enc))
            m_encodingMode = DisplayCodeGenerator::EncodingMode::Mono1Bit;
    }

    if (m_colorMode != DisplayProfile::Mono1Bit
        && m_monoLayout != DisplayCodeGenerator::MonoLayout::RowPacked) {
        m_monoLayout = DisplayCodeGenerator::MonoLayout::RowPacked;
        emit monoLayoutChanged();
    }

    emit encodingModeChanged();
    emit colorModeChanged();
    syncProfileIdFromDimensions();
    scheduleRebuild();
}

void DisplayConverter::setScaleMode(int mode)
{
    const auto m = static_cast<DisplayProfile::ScaleMode>(qBound(0, mode, 2));
    if (m_scaleMode == m)
        return;
    m_scaleMode = m;
    emit scaleModeChanged();
    scheduleRebuild();
}

void DisplayConverter::setDithering(bool on)
{
    const auto mode = on
        ? ImageFiltersPipeline::DitherMode::FloydSteinberg
        : ImageFiltersPipeline::DitherMode::None;
    if (m_dithering == on && m_filterParams.ditherMode == mode)
        return;
    m_dithering = on;
    m_filterParams.ditherMode = mode;
    emit ditheringChanged();
    emit ditherModeChanged();
    scheduleRebuild();
}

void DisplayConverter::setArrayName(const QString &name)
{
    const QString safe = DisplayCodeGenerator::sanitizeIdentifier(name);
    if (m_arrayName == safe)
        return;
    m_arrayName = safe;
    emit arrayNameChanged();
    schedulePersistSession();
    if (hasImage())
        scheduleRebuild();
}

int DisplayConverter::dataByteCount() const
{
    return DisplayCodeGenerator::flashFootprintBytes(m_encodingMode,
                                                     m_displayWidth,
                                                     m_displayHeight,
                                                     m_lastResult.monoBits,
                                                     m_lastResult.monoBuffer,
                                                     m_lastResult.grayscale8,
                                                     m_lastResult.rgb565,
                                                     m_lastResult.rgb888,
                                                     m_lastResult.rgb233,
                                                     m_lastResult.rgb24,
                                                     m_monoLayout,
                                                     m_codeGenOptions,
                                                     m_lastResult.indexedPalette);
}

QString DisplayConverter::colorModeName() const
{
    return m_colorMode == DisplayProfile::Rgb565
        ? AppLocale::tr("Color")
        : AppLocale::tr("B&W");
}

QString DisplayConverter::encodingModeName() const
{
    const QVariantList modes = availableEncodingModes();
    for (const QVariant &v : modes) {
        const QVariantMap m = v.toMap();
        if (m.value(QStringLiteral("mode")).toInt() == static_cast<int>(m_encodingMode))
            return m.value(QStringLiteral("name")).toString();
    }
    return AppLocale::tr("Unknown");
}

bool DisplayConverter::encodingIsMono1Bit() const
{
    return encodingIsMonoMode(static_cast<int>(m_encodingMode));
}

bool DisplayConverter::encodingIsGrayscale() const
{
    return PixelFormatCatalog::isGrayscale(m_encodingMode);
}

bool DisplayConverter::encodingIsColor() const
{
    return encodingIsColorMode(static_cast<int>(m_encodingMode));
}

QString DisplayConverter::monoLayoutName() const
{
    using Layout = DisplayCodeGenerator::MonoLayout;
    if (m_monoLayout == Layout::Ssd1306Page)
        return QCoreApplication::translate("PixelStudio", "Vertical page buffer");
    if (m_monoLayout == Layout::VerticalColumn)
        return QCoreApplication::translate("PixelStudio", "Vertical column");
    return QCoreApplication::translate("PixelStudio", "Row-packed");
}

QVariantList DisplayConverter::previewPalette() const
{
    if (!m_lastResult.indexedPalette.isEmpty())
        return paletteFromRgbList(m_lastResult.indexedPalette);
    if (!m_lastResult.preview.isNull())
        return paletteFromPreviewImage(m_lastResult.preview);
    return {};
}

int DisplayConverter::previewColorCount() const
{
    return previewPalette().size();
}

QString DisplayConverter::imageFormatName() const
{
    if (!hasImage())
        return QString();
    const QString path = m_sourcePath.toLocalFile();
    const int dot = path.lastIndexOf(QLatin1Char('.'));
    if (dot >= 0 && dot < path.size() - 1)
        return path.mid(dot + 1).toUpper();
    return QCoreApplication::translate("PixelStudio", "Image");
}

int DisplayConverter::sourceWidth() const
{
    const QImage img = orientedSource();
    return img.isNull() ? 0 : img.width();
}

int DisplayConverter::sourceHeight() const
{
    const QImage img = orientedSource();
    return img.isNull() ? 0 : img.height();
}

void DisplayConverter::setRotation(int degrees)
{
    degrees = ((degrees % 360) + 360) % 360;
    if (degrees != 0 && degrees != 90 && degrees != 180 && degrees != 270)
        degrees = 0;
    if (m_rotation == degrees)
        return;
    m_rotation = degrees;
    markOrientedDirty();
    emit rotationChanged();
    if (hasImage()) {
        refreshSourcePreview();
        emit sourceWidthChanged();
        emit sourceHeightChanged();
        scheduleRebuild();
    }
}

void DisplayConverter::rotateClockwise()
{
    setRotation((m_rotation + 90) % 360);
}

QImage DisplayConverter::orientedSource() const
{
    if (m_sourceImage.isNull())
        return {};
    if (!m_orientedDirty && !m_orientedCache.isNull())
        return m_orientedCache;

    m_orientedCache = ConvertPipeline::applyOrientation(m_sourceImage, pipelineParams());
    m_orientedDirty = false;
    return m_orientedCache;
}

void DisplayConverter::setFlipHorizontal(bool on)
{
    if (m_flipHorizontal == on)
        return;
    m_flipHorizontal = on;
    markOrientedDirty();
    emit flipHorizontalChanged();
    if (hasImage()) {
        refreshSourcePreview();
        emit sourceWidthChanged();
        emit sourceHeightChanged();
        scheduleRebuild();
    }
}

void DisplayConverter::setFlipVertical(bool on)
{
    if (m_flipVertical == on)
        return;
    m_flipVertical = on;
    markOrientedDirty();
    emit flipVerticalChanged();
    if (hasImage()) {
        refreshSourcePreview();
        emit sourceWidthChanged();
        emit sourceHeightChanged();
        scheduleRebuild();
    }
}

void DisplayConverter::setInvertMono(bool on)
{
    if (m_invertMono == on)
        return;
    m_invertMono = on;
    emit invertMonoChanged();
    if (hasImage())
        scheduleRebuild();
}

void DisplayConverter::setFilterInvert(bool on)
{
    if (m_filterParams.invert == on)
        return;
    m_filterParams.invert = on;
    emit filterInvertChanged();
    if (hasImage())
        scheduleRebuild();
}

void DisplayConverter::setMonoLayout(int mode)
{
    const auto next = static_cast<DisplayCodeGenerator::MonoLayout>(qBound(0, mode, 2));
    if (m_monoLayout == next)
        return;
    m_monoLayout = next;
    emit monoLayoutChanged();
    if (hasImage() && encodingIsMono1Bit())
        scheduleRebuild();
}

void DisplayConverter::setEncodingMode(int mode)
{
    const auto next = static_cast<DisplayCodeGenerator::EncodingMode>(
        qBound(0, mode, static_cast<int>(DisplayCodeGenerator::EncodingMode::Count) - 1));
    if (m_encodingMode == next)
        return;
    m_encodingMode = next;

    const auto nextColor = encodingIsColorMode(static_cast<int>(next))
                               ? DisplayProfile::Rgb565
                               : DisplayProfile::Mono1Bit;
    if (m_colorMode != nextColor) {
        m_colorMode = nextColor;
        emit colorModeChanged();
    }
    if (m_colorMode != DisplayProfile::Mono1Bit
        && m_monoLayout != DisplayCodeGenerator::MonoLayout::RowPacked) {
        m_monoLayout = DisplayCodeGenerator::MonoLayout::RowPacked;
        emit monoLayoutChanged();
    }

    emit encodingModeChanged();
    if (hasImage())
        scheduleRebuild();
}

void DisplayConverter::swapDisplayDimensions()
{
    const int w = m_displayWidth;
    const int h = m_displayHeight;
    if (w == h)
        return;
    m_displayWidth = h;
    m_displayHeight = w;
    emit displayWidthChanged();
    emit displayHeightChanged();
    syncProfileIdFromDimensions();
    scheduleRebuild();
}

void DisplayConverter::refreshSourcePreview()
{
    if (m_sourceImage.isNull())
        return;
    m_sourcePath = writeTempPreview(QStringLiteral("source"), orientedSource());
    emit sourcePathChanged();
}

void DisplayConverter::setMonoThreshold(int value)
{
    value = qBound(0, value, 255);
    if (m_monoThreshold == value)
        return;
    m_monoThreshold = value;
    m_filterParams.threshold = value;
    emit monoThresholdChanged();
    if (hasImage() && encodingIsMono1Bit())
        scheduleRebuild();
}

void DisplayConverter::setBlackBackground(bool on)
{
    if (m_filterParams.blackBackground == on)
        return;
    m_filterParams.blackBackground = on;
    emit blackBackgroundChanged();
    scheduleRebuild();
}

void DisplayConverter::setBrightness(int value)
{
    value = qBound(0, value, 200);
    if (m_filterParams.brightness == value)
        return;
    m_filterParams.brightness = value;
    markToneCustom();
    emit brightnessChanged();
    scheduleRebuild();
}

void DisplayConverter::setContrast(int value)
{
    value = qBound(0, value, 200);
    if (m_filterParams.contrast == value)
        return;
    m_filterParams.contrast = value;
    markToneCustom();
    emit contrastChanged();
    scheduleRebuild();
}

void DisplayConverter::setSaturation(int value)
{
    value = qBound(0, value, 200);
    if (m_filterParams.saturation == value)
        return;
    m_filterParams.saturation = value;
    markToneCustom();
    emit saturationChanged();
    scheduleRebuild();
}

void DisplayConverter::setExposure(int value)
{
    value = qBound(50, value, 200);
    if (m_filterParams.exposure == value)
        return;
    m_filterParams.exposure = value;
    markToneCustom();
    emit exposureChanged();
    scheduleRebuild();
}

void DisplayConverter::setGamma(int value)
{
    value = qBound(50, value, 200);
    if (m_filterParams.gamma == value)
        return;
    m_filterParams.gamma = value;
    markToneCustom();
    emit gammaChanged();
    scheduleRebuild();
}

void DisplayConverter::setBlur(int value)
{
    value = qBound(0, value, 6);
    if (m_filterParams.blur == value)
        return;
    m_filterParams.blur = value;
    emit blurChanged();
    scheduleRebuild();
}

void DisplayConverter::setPosterizeRgb(int value)
{
    value = qBound(0, value, 30);
    if (m_filterParams.posterizeRgb == value)
        return;
    m_filterParams.posterizeRgb = value;
    emit posterizeRgbChanged();
    scheduleRebuild();
}

void DisplayConverter::setColorMaskEnabled(bool on)
{
    if (m_filterParams.colorMaskEnabled == on)
        return;
    m_filterParams.colorMaskEnabled = on;
    emit colorMaskEnabledChanged();
    scheduleRebuild();
}

void DisplayConverter::setMaskColor(const QColor &color)
{
    const QColor safe = color.isValid() ? color : QColor(Qt::black);
    if (m_filterParams.maskColor == safe)
        return;
    m_filterParams.maskColor = safe;
    emit maskColorChanged();
    scheduleRebuild();
}

void DisplayConverter::setMaskTolerance(int value)
{
    value = qBound(0, value, 255);
    if (m_filterParams.maskTolerance == value)
        return;
    m_filterParams.maskTolerance = value;
    emit maskToleranceChanged();
    scheduleRebuild();
}

void DisplayConverter::setMaskAmplify(int value)
{
    value = qBound(1, value, 10);
    if (m_filterParams.maskAmplify == value)
        return;
    m_filterParams.maskAmplify = value;
    emit maskAmplifyChanged();
    scheduleRebuild();
}

void DisplayConverter::setSharpen(bool on)
{
    if (m_filterParams.sharpen == on)
        return;
    m_filterParams.sharpen = on;
    emit sharpenChanged();
    scheduleRebuild();
}

void DisplayConverter::setSobelEdges(int value)
{
    value = qBound(0, value, 100);
    if (m_filterParams.sobelEdges == value)
        return;
    m_filterParams.sobelEdges = value;
    emit sobelEdgesChanged();
    scheduleRebuild();
}

void DisplayConverter::setPosterizeGray(int value)
{
    value = qBound(0, value, 30);
    if (m_filterParams.posterizeGray == value)
        return;
    m_filterParams.posterizeGray = value;
    emit posterizeGrayChanged();
    scheduleRebuild();
}

void DisplayConverter::setDitherMode(int mode)
{
    const auto next = mode == static_cast<int>(ImageFiltersPipeline::DitherMode::FloydSteinberg)
        ? ImageFiltersPipeline::DitherMode::FloydSteinberg
        : (mode == static_cast<int>(ImageFiltersPipeline::DitherMode::Jjn)
            ? ImageFiltersPipeline::DitherMode::Jjn
            : (mode == static_cast<int>(ImageFiltersPipeline::DitherMode::Bayer)
                ? ImageFiltersPipeline::DitherMode::Bayer
                : ImageFiltersPipeline::DitherMode::None));
    if (m_filterParams.ditherMode == next)
        return;
    m_filterParams.ditherMode = next;
    m_dithering = (next == ImageFiltersPipeline::DitherMode::FloydSteinberg);
    emit ditherModeChanged();
    emit ditheringChanged();
    scheduleRebuild();
}

void DisplayConverter::setContourMode(int mode)
{
    const auto next = mode == static_cast<int>(ImageFiltersPipeline::ContourMode::FourDir)
        ? ImageFiltersPipeline::ContourMode::FourDir
        : (mode == static_cast<int>(ImageFiltersPipeline::ContourMode::EightDir)
            ? ImageFiltersPipeline::ContourMode::EightDir
            : ImageFiltersPipeline::ContourMode::None);
    if (m_filterParams.contourMode == next)
        return;
    m_filterParams.contourMode = next;
    emit contourModeChanged();
    scheduleRebuild();
}

namespace {

QSize fittedContentSize(const QImage &oriented, int tw, int th, DisplayProfile::ScaleMode mode)
{
    if (oriented.isNull() || tw < 1 || th < 1)
        return {};

    if (mode == DisplayProfile::Stretch || mode == DisplayProfile::Crop)
        return QSize(tw, th);

    const qreal sw = oriented.width();
    const qreal sh = oriented.height();
    if (sw < 1 || sh < 1)
        return {};
    const qreal scale = qMin(qreal(tw) / sw, qreal(th) / sh);
    return QSize(qMax(1, qRound(sw * scale)), qMax(1, qRound(sh * scale)));
}

} // namespace

void DisplayConverter::markToneCustom()
{
    if (m_filterParams.tonePreset == ImageFiltersPipeline::TonePreset::Custom)
        return;
    m_filterParams.tonePreset = ImageFiltersPipeline::TonePreset::Custom;
    emit tonePresetChanged();
}

void DisplayConverter::emitAllFilterSignals()
{
    emit blackBackgroundChanged();
    emit brightnessChanged();
    emit contrastChanged();
    emit saturationChanged();
    emit exposureChanged();
    emit gammaChanged();
    emit blurChanged();
    emit posterizeRgbChanged();
    emit colorMaskEnabledChanged();
    emit maskColorChanged();
    emit maskToleranceChanged();
    emit maskAmplifyChanged();
    emit sharpenChanged();
    emit sobelEdgesChanged();
    emit posterizeGrayChanged();
    emit ditherModeChanged();
    emit contourModeChanged();
    emit filterInvertChanged();
    emit tonePresetChanged();
}

DisplayCodeGenerator::CodeGenOptions DisplayConverter::codeGenOptions() const
{
    return m_codeGenOptions;
}

void DisplayConverter::setTonePreset(int preset)
{
    const auto next = static_cast<ImageFiltersPipeline::TonePreset>(qBound(0, preset, 2));
    const ImageFiltersPipeline::Params tone = ImageFiltersPipeline::paramsForTonePreset(next);
    m_filterParams.tonePreset = next;
    if (next != ImageFiltersPipeline::TonePreset::Custom) {
        m_filterParams.brightness = tone.brightness;
        m_filterParams.contrast = tone.contrast;
        m_filterParams.saturation = tone.saturation;
        m_filterParams.exposure = tone.exposure;
        m_filterParams.gamma = tone.gamma;
        m_filterParams.posterizeGray = tone.posterizeGray;
    }
    emit brightnessChanged();
    emit contrastChanged();
    emit saturationChanged();
    emit exposureChanged();
    emit gammaChanged();
    emit posterizeGrayChanged();
    emit tonePresetChanged();
    scheduleRebuild();
}

void DisplayConverter::resetFilters()
{
    const int threshold = m_monoThreshold;
    const bool invert = m_invertMono;
    m_filterParams = ImageFiltersPipeline::Params{};
    m_filterParams.threshold = threshold;
    m_dithering = true;
    m_filterParams.ditherMode = ImageFiltersPipeline::DitherMode::FloydSteinberg;
    m_invertMono = invert;
    m_filterParams.invert = false;
    emit ditheringChanged();
    emit invertMonoChanged();
    emitAllFilterSignals();
    scheduleRebuild();
}

void DisplayConverter::resetTransform()
{
    m_rotation = 0;
    m_flipHorizontal = false;
    m_flipVertical = false;
    m_scaleMode = DisplayProfile::Fit;
    m_offsetX = 0;
    m_offsetY = 0;
    emit rotationChanged();
    emit flipHorizontalChanged();
    emit flipVerticalChanged();
    emit scaleModeChanged();
    emit offsetChanged();
    markOrientedDirty();
    scheduleRebuild();
}

void DisplayConverter::centerOffsetOnDisplay()
{
    if (!hasImage())
        return;
    const QSize content = fittedContentSize(orientedSource(),
                                            m_displayWidth,
                                            m_displayHeight,
                                            m_scaleMode);
    if (content.isEmpty())
        return;
    setOffsetX((m_displayWidth - content.width()) / 2);
    setOffsetY((m_displayHeight - content.height()) / 2);
}

void DisplayConverter::copyGeneratedArray()
{
    copyToClipboard(DisplayCodeGenerator::extractArrayBody(m_generatedCode));
}

void DisplayConverter::setCodeIncludeComments(bool on)
{
    if (m_codeGenOptions.includeHeaderComments == on)
        return;
    m_codeGenOptions.includeHeaderComments = on;
    emit codeGenOptionsChanged();
    scheduleRebuild(true);
}

void DisplayConverter::setCodeUseProgmem(bool on)
{
    if (m_codeGenOptions.useProgmem == on)
        return;
    m_codeGenOptions.useProgmem = on;
    emit codeGenOptionsChanged();
    scheduleRebuild(true);
}

void DisplayConverter::setCodeStaticStorage(bool on)
{
    if (m_codeGenOptions.staticStorage == on)
        return;
    m_codeGenOptions.staticStorage = on;
    emit codeGenOptionsChanged();
    scheduleRebuild(true);
}

void DisplayConverter::setRgb565BigEndian(bool on)
{
    if (m_codeGenOptions.rgb565BigEndian == on)
        return;
    m_codeGenOptions.rgb565BigEndian = on;
    emit rgb565BigEndianChanged();
    emit codeGenOptionsChanged();
    scheduleRebuild(true);
}

void DisplayConverter::setCodeDmaAlign(int align)
{
    align = align == 8 ? 8 : (align == 4 ? 4 : 0);
    if (m_codeGenOptions.dmaPaddingAlign == align)
        return;
    m_codeGenOptions.dmaPaddingAlign = align;
    emit codeDmaAlignChanged();
    emit codeGenOptionsChanged();
    scheduleRebuild(true);
}

void DisplayConverter::setLinearColorSpace(bool on)
{
    if (m_linearColorSpace == on)
        return;
    m_linearColorSpace = on;
    emit linearColorSpaceChanged();
    if (hasImage())
        scheduleRebuild();
}

void DisplayConverter::setShowGrid(bool on)
{
    if (m_showGrid == on)
        return;
    m_showGrid = on;
    emit showGridChanged();
}

void DisplayConverter::setGridThresholdZoom(int value)
{
    value = qBound(1, value, 64);
    if (m_gridThresholdZoom == value)
        return;
    m_gridThresholdZoom = value;
    emit gridThresholdZoomChanged();
}

void DisplayConverter::syncProfileIdFromDimensions()
{
    for (const DisplayProfile &p : DisplayProfile::presets()) {
        if (p.id == QStringLiteral("custom"))
            continue;
        if (p.width == m_displayWidth && p.height == m_displayHeight) {
            if (m_profileId != p.id) {
                m_profileId = p.id;
                emit profileIdChanged();
            }
            return;
        }
    }
    if (m_profileId != QStringLiteral("custom")) {
        m_profileId = QStringLiteral("custom");
        emit profileIdChanged();
    }
}

void DisplayConverter::applyProfile(const DisplayProfile &profile)
{
    if (profile.id == QStringLiteral("custom"))
        return;

    m_displayWidth = profile.width;
    m_displayHeight = profile.height;
    emit displayWidthChanged();
    emit displayHeightChanged();
}

bool DisplayConverter::loadImage(const QUrl &url)
{
    if (url.scheme().startsWith(QStringLiteral("http"), Qt::CaseInsensitive)) {
        loadFromUrl(url.toString());
        return true;
    }
    QImage img;
    if (!m_loader->loadFromFile(url, img))
        return false;
    onImageLoaded(img, url);
    if (m_session)
        m_session->addRecentFile(url);
    rememberOpenImageDir(url.toLocalFile().isEmpty() ? url.path() : url.toLocalFile());
    return true;
}

bool DisplayConverter::loadRecentFile(const QString &localPath)
{
    if (localPath.isEmpty())
        return false;
    const QFileInfo info(localPath);
    if (info.suffix().compare(QStringLiteral("json"), Qt::CaseInsensitive) == 0)
        return openProject(QUrl::fromLocalFile(localPath));
    return loadImage(QUrl::fromLocalFile(localPath));
}

bool DisplayConverter::loadFromClipboard()
{
    QImage img;
    if (!m_loader->loadFromClipboard(img))
        return false;
    onImageLoaded(img, QUrl(QStringLiteral("clipboard://image")));
    return true;
}

void DisplayConverter::loadFromUrl(const QString &urlString)
{
    m_loader->loadFromUrl(urlString);
}

void DisplayConverter::clear()
{
    m_rebuildDebounceTimer.stop();
    m_rebuildPending = false;
    m_lastAppliedGeneration = ++m_nextGeneration;
    m_batchService.reset();
    m_sourceImage = QImage();
    m_sourceFilePath.clear();
    m_sourcePath.clear();
    m_previewPath.clear();
    m_processPreviewPath.clear();
    m_generatedCode.clear();
    m_flashReport.clear();
    m_lastResult = {};
    m_rotation = 0;
    m_flipHorizontal = false;
    m_flipVertical = false;
    m_invertMono = false;
    m_filterParams = ImageFiltersPipeline::Params{};
    m_filterParams.threshold = m_monoThreshold;
    m_filterParams.ditherMode = m_dithering
        ? ImageFiltersPipeline::DitherMode::FloydSteinberg
        : ImageFiltersPipeline::DitherMode::None;
    markOrientedDirty();
    emit sourcePathChanged();
    emit previewPathChanged();
    emit processPreviewPathChanged();
    emit hasImageChanged();
    emit sourceWidthChanged();
    emit sourceHeightChanged();
    emit generatedCodeChanged();
    emit flashReportChanged();
    emit rotationChanged();
    emit flipHorizontalChanged();
    emit flipVerticalChanged();
    emit invertMonoChanged();
    emit filterInvertChanged();
    emit blackBackgroundChanged();
    emit brightnessChanged();
    emit contrastChanged();
    emit saturationChanged();
    emit exposureChanged();
    emit gammaChanged();
    emit blurChanged();
    emit posterizeRgbChanged();
    emit colorMaskEnabledChanged();
    emit maskColorChanged();
    emit maskToleranceChanged();
    emit maskAmplifyChanged();
    emit sharpenChanged();
    emit sobelEdgesChanged();
    emit posterizeGrayChanged();
    emit ditherModeChanged();
    emit contourModeChanged();
    emit tonePresetChanged();
    emit codeGenOptionsChanged();
    emit batchRunningChanged();
    emit batchProgressChanged();
}

void DisplayConverter::refresh()
{
    rebuild();
}

QVariantList DisplayConverter::displayPresets() const
{
    QVariantList list;
    for (const DisplayProfile &p : DisplayProfile::presets()) {
        QVariantMap m;
        m[QStringLiteral("id")] = p.id;
        m[QStringLiteral("name")] = QCoreApplication::translate("PixelStudio", p.name.toUtf8().constData());
        m[QStringLiteral("width")] = p.width;
        m[QStringLiteral("height")] = p.height;
        list.append(m);
    }
    return list;
}

QVariantList DisplayConverter::availableEncodingModes() const
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

QVariantList DisplayConverter::availableEncodingModesForUi() const
{
    return availableEncodingModes();
}

QVariantList DisplayConverter::availableBasicEncodingModes() const
{
    using Mode = DisplayCodeGenerator::EncodingMode;
    using Layout = DisplayCodeGenerator::MonoLayout;
    QVariantList list;
    if (m_colorMode == DisplayProfile::Rgb565) {
        list.append(QVariantMap{
            {QStringLiteral("name"), AppLocale::tr("RGB565")},
            {QStringLiteral("mode"), static_cast<int>(Mode::Rgb565)},
            {QStringLiteral("monoLayout"), static_cast<int>(Layout::RowPacked)},
        });
        list.append(QVariantMap{
            {QStringLiteral("name"), AppLocale::tr("Indexed color (8-bit palette)")},
            {QStringLiteral("mode"), static_cast<int>(Mode::Indexed8)},
            {QStringLiteral("monoLayout"), static_cast<int>(Layout::RowPacked)},
        });
        return list;
    }
    list.append(QVariantMap{
        {QStringLiteral("name"), AppLocale::tr("Standard row")},
        {QStringLiteral("mode"), static_cast<int>(Mode::Mono1Bit)},
        {QStringLiteral("monoLayout"), static_cast<int>(Layout::RowPacked)},
    });
    list.append(QVariantMap{
        {QStringLiteral("name"), AppLocale::tr("Vertical page buffer")},
        {QStringLiteral("mode"), static_cast<int>(Mode::Mono1Bit)},
        {QStringLiteral("monoLayout"), static_cast<int>(Layout::Ssd1306Page)},
    });
    list.append(QVariantMap{
        {QStringLiteral("name"), AppLocale::tr("Vertical column")},
        {QStringLiteral("mode"), static_cast<int>(Mode::Mono1Bit)},
        {QStringLiteral("monoLayout"), static_cast<int>(Layout::VerticalColumn)},
    });
    list.append(QVariantMap{
        {QStringLiteral("name"), AppLocale::tr("Grayscale (4-bit)")},
        {QStringLiteral("mode"), static_cast<int>(Mode::Grayscale4)},
        {QStringLiteral("monoLayout"), static_cast<int>(Layout::RowPacked)},
    });
    return list;
}

void DisplayConverter::applyBasicEncoding(int mode, int monoLayout)
{
    setEncodingMode(mode);
    if (encodingIsMono1Bit())
        setMonoLayout(monoLayout);
}

QVariantList DisplayConverter::workflowPresets() const
{
    QVariantList list = ControllerCatalog::workflowPresets();
    for (QVariant &item : list) {
        QVariantMap m = item.toMap();
        for (const char *key : {"name", "description"}) {
            const QString text = m.value(QString::fromLatin1(key)).toString();
            if (!text.isEmpty())
                m.insert(QString::fromLatin1(key), AppLocale::tr(text.toUtf8().constData()));
        }
        item = m;
    }
    return list;
}

void DisplayConverter::applyWorkflowPreset(const QString &id)
{
    if (id == QStringLiteral("icon")) {
        setProfileId(QStringLiteral("128x64"));
        setColorMode(static_cast<int>(DisplayProfile::Mono1Bit));
        m_scaleMode = DisplayProfile::Crop;
        m_encodingMode = DisplayCodeGenerator::EncodingMode::Mono1Bit;
    } else if (id == QStringLiteral("splash")) {
        setProfileId(QStringLiteral("240x240"));
        setColorMode(static_cast<int>(DisplayProfile::Rgb565));
        m_scaleMode = DisplayProfile::Crop;
    } else if (id == QStringLiteral("epaper")) {
        setProfileId(QStringLiteral("250x122"));
        setColorMode(static_cast<int>(DisplayProfile::Mono1Bit));
        m_filterParams.contrast = 140;
        m_filterParams.ditherMode = ImageFiltersPipeline::DitherMode::FloydSteinberg;
    } else if (id == QStringLiteral("indexed")) {
        setProfileId(QStringLiteral("240x240"));
        setColorMode(static_cast<int>(DisplayProfile::Rgb565));
        m_encodingMode = DisplayCodeGenerator::EncodingMode::Indexed8;
        m_filterParams.posterizeRgb = 6;
    } else {
        return;
    }
    emit scaleModeChanged();
    emit encodingModeChanged();
    emit contrastChanged();
    emit ditherModeChanged();
    emit posterizeRgbChanged();
    scheduleRebuild();
}

void DisplayConverter::newProject(const QString &name)
{
    const SessionSnapshot defaults = SessionSettings::defaultSnapshot();
    applySessionSnapshot(defaults);
    m_offsetX = 0;
    m_offsetY = 0;
    emit offsetChanged();
    m_project = ProjectService::fromSession(name, defaults, 0, 0);
    m_projectFile = QUrl();
    updateWatchExportPrefix();
    restartAutosaveTimer();
    emit projectChanged();
}

bool DisplayConverter::openProject(const QUrl &url)
{
    StudioProject project;
    QString error;
    if (!ProjectService::load(url, &project, &error)) {
        emit errorOccurred(error);
        return false;
    }
    applyProject(project);
    m_projectFile = url;
    const QString localPath = url.toLocalFile();
    if (!AppPaths::isExcludedFromRecentPath(localPath) && m_session)
        m_session->addRecentFile(url);
    if (!AppPaths::isInternalDataPath(localPath))
        m_uiState.lastProjectFile = localPath;
    updateWatchExportPrefix();
    restartAutosaveTimer();
    schedulePersistSession();
    emit projectChanged();
    return true;
}

bool DisplayConverter::saveProject()
{
    if (m_projectFile.isEmpty()) {
        emit errorOccurred(AppLocale::tr("Specify a project path"));
        return false;
    }
    return saveProjectAs(m_projectFile);
}

bool DisplayConverter::saveProjectAs(const QUrl &url)
{
    const QString previousPath = m_projectFile.toLocalFile();
    m_project = projectSnapshot();
    QString error;
    if (!ProjectService::save(m_project, url, &error)) {
        emit errorOccurred(error);
        return false;
    }
    m_projectFile = url;
    const QString localPath = url.toLocalFile();
    if (!AppPaths::isExcludedFromRecentPath(localPath) && m_session)
        m_session->addRecentFile(url);
    if (!previousPath.isEmpty() && AppPaths::isTabCachePath(previousPath)
        && !AppPaths::isTabCachePath(localPath) && m_session) {
        m_session->removeRecentPath(previousPath);
    }
    if (!AppPaths::isInternalDataPath(localPath))
        m_uiState.lastProjectFile = localPath;
    schedulePersistSession();
    updateWatchExportPrefix();
    restartAutosaveTimer();
    emit projectChanged();
    return true;
}

bool DisplayConverter::importHeader(const QUrl &url)
{
    const HeaderImportResult result = HeaderImportService::importHeader(url);
    if (!result.ok) {
        emit errorOccurred(result.errorMessage);
        return false;
    }
    m_sourceImage = result.preview;
    m_arrayName = result.arrayName;
    m_displayWidth = result.width;
    m_displayHeight = result.height;
    m_colorMode = result.colorMode;
    m_encodingMode = result.encodingMode;
    m_profileId = QStringLiteral("custom");
    markOrientedDirty();
    refreshSourcePreview();
    emit hasImageChanged();
    emit arrayNameChanged();
    emit displayWidthChanged();
    emit displayHeightChanged();
    emit colorModeChanged();
    emit encodingModeChanged();
    scheduleRebuild(true);
    return true;
}

void DisplayConverter::configureWatchFolder(const QString &inputFolder, const QString &outputFolder)
{
    QString out = outputFolder.trimmed();
    if (out.isEmpty())
        out = AppPaths::watchDir();
    m_uiState.watchInputFolder = inputFolder;
    m_uiState.watchOutputFolder = out;
    m_watchService.configure(inputFolder, out);
    updateWatchExportPrefix();
    schedulePersistSession();
}

void DisplayConverter::configureWatchFolders(const QUrl &inputFolder, const QUrl &outputFolder)
{
    configureWatchFolder(inputFolder.toLocalFile(), outputFolder.toLocalFile());
}

void DisplayConverter::setWatchFolderActive(bool active)
{
    m_uiState.watchActive = active;
    m_watchService.setActive(active);
    schedulePersistSession();
}

bool DisplayConverter::buildSpriteAtlas(const QVariantList &urls, const QUrl &targetFile, int frameWidth, int frameHeight)
{
    SpriteAtlasRequest request;
    request.pipeline = pipelineParams();
    request.arrayPrefix = m_arrayName.isEmpty() ? QStringLiteral("sprite") : m_arrayName;
    request.frameWidth = frameWidth;
    request.frameHeight = frameHeight;
    request.fixedGrid = true;
    request.padding = 1;
    request.encodingMode = m_encodingMode;
    request.monoLayout = m_monoLayout;
    request.codeGenOptions = m_codeGenOptions;
    for (const QVariant &value : urls) {
        const QUrl url = value.toUrl();
        if (url.isValid())
            request.files.append(url);
    }
    const SpriteAtlasResult result = SpriteAtlasService::build(request);
    if (!result.ok) {
        emit errorOccurred(result.errorMessage);
        return false;
    }
    QString path = targetFile.toLocalFile();
    if (path.isEmpty())
        path = targetFile.path();
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        emit errorOccurred(AppLocale::tr("Failed to write atlas: %1").arg(path));
        return false;
    }
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << result.headerCode;
    return true;
}

void DisplayConverter::copyToClipboard(const QString &text)
{
    QGuiApplication::clipboard()->setText(text);
}

bool DisplayConverter::saveCodeToFile(const QUrl &url)
{
    if (m_generatedCode.isEmpty())
        return false;

    QString path = url.toLocalFile();
    if (path.isEmpty())
        path = url.path();
    if (path.isEmpty()) {
        emit errorOccurred(AppLocale::tr("Specify a file path"));
        return false;
    }

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        emit errorOccurred(AppLocale::tr("Failed to write file: %1").arg(path));
        return false;
    }
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << m_generatedCode;
    rememberExportDir(path);
    if (m_session)
        m_session->addRecentExport(path);
    return true;
}

bool DisplayConverter::saveBinaryToFile(const QUrl &url)
{
    if (m_lastResult.width < 1 || m_lastResult.height < 1)
        return false;
    QString path = url.toLocalFile();
    if (path.isEmpty())
        path = url.path();
    if (path.isEmpty()) {
        emit errorOccurred(AppLocale::tr("Specify a file path"));
        return false;
    }

    const QByteArray data = DisplayCodeGenerator::binaryData(
        m_encodingMode,
        m_displayWidth,
        m_displayHeight,
        m_lastResult.monoBits,
        m_lastResult.monoBuffer,
        m_lastResult.grayscale8,
        m_lastResult.rgb565,
        m_lastResult.rgb888,
        m_lastResult.rgb233,
        m_lastResult.rgb24,
        m_monoLayout,
        m_codeGenOptions);

    QString error;
    if (!BinaryExporter::save(path, data, &error)) {
        emit errorOccurred(error);
        return false;
    }
    rememberExportDir(path);
    if (m_session)
        m_session->addRecentExport(path);
    return true;
}

void DisplayConverter::enqueueBatchCodeExport(const QVariantList &urls, const QUrl &targetFile)
{
    QVector<QUrl> files;
    files.reserve(urls.size());
    for (const QVariant &entry : urls) {
        const QUrl url = entry.toUrl();
        if (!url.isValid())
            continue;
        files.append(url);
    }
    if (files.isEmpty()) {
        emit errorOccurred(AppLocale::tr("Batch file list is empty"));
        return;
    }
    BatchExportJob job;
    job.files = files;
    job.targetFile = targetFile;
    job.pipeline = pipelineParams();
    job.profileId = m_profileId;
    job.encodingMode = m_encodingMode;
    job.monoLayout = m_monoLayout;
    job.codeGenOptions = m_codeGenOptions;
    m_batchService.enqueue(job);
}

void DisplayConverter::cancelBatchExport()
{
    m_batchService.cancel();
}

void DisplayConverter::onImageLoaded(const QImage &image, const QUrl &sourceUrl)
{
    m_sourceImage = image;
    m_rotation = 0;
    m_flipHorizontal = false;
    m_flipVertical = false;
    markOrientedDirty();
    if (sourceUrl.isLocalFile()) {
        const QString local = sourceUrl.toLocalFile();
        m_sourceFilePath = local.isEmpty() ? sourceUrl.path() : local;
    } else {
        m_sourceFilePath.clear();
    }
    refreshSourcePreview();
    emit hasImageChanged();
    emit rotationChanged();
    emit flipHorizontalChanged();
    emit flipVerticalChanged();
    emit sourceWidthChanged();
    emit sourceHeightChanged();
    scheduleRebuild(true);
}

void DisplayConverter::onBatchFinished(bool ok, const QString &errorMessage)
{
    if (!ok && !errorMessage.isEmpty())
        emit errorOccurred(errorMessage);
}

void DisplayConverter::rebuild()
{
    scheduleRebuild(true);
}

void DisplayConverter::scheduleRebuild(bool immediate)
{
    schedulePersistSession();
    if (immediate) {
        m_rebuildDebounceTimer.stop();
        startAsyncRebuild();
        return;
    }
    m_rebuildDebounceTimer.start();
}

void DisplayConverter::startAsyncRebuild()
{
    if (m_rebuildWatcher.isRunning()) {
        m_rebuildPending = true;
        return;
    }

    if (m_sourceImage.isNull()) {
        m_previewPath.clear();
        m_processPreviewPath.clear();
        m_generatedCode.clear();
        emit previewPathChanged();
        emit processPreviewPathChanged();
        emit generatedCodeChanged();
        return;
    }

    const QImage source = orientedSource();
    const int width = m_displayWidth;
    const int height = m_displayHeight;
    const auto colorMode = m_colorMode;
    const auto scaleMode = m_scaleMode;
    const int monoThreshold = m_monoThreshold;
    const bool invertMono = m_invertMono;
    const ImageFiltersPipeline::Params filterParams = m_filterParams;
    const QString profileId = m_profileId;
    const QString arrayName = m_arrayName;
    const auto encodingMode = m_encodingMode;
    const auto monoLayout = m_monoLayout;
    const DisplayCodeGenerator::CodeGenOptions codeOptions = m_codeGenOptions;
    const bool linearColorSpace = m_linearColorSpace;
    const quint64 generation = ++m_nextGeneration;

    auto future = QtConcurrent::run([source,
                                     width,
                                     height,
                                     colorMode,
                                     scaleMode,
                                     monoThreshold,
                                     invertMono,
                                     filterParams,
                                     profileId,
                                     arrayName,
                                     encodingMode,
                                     monoLayout,
                                     codeOptions,
                                     linearColorSpace,
                                     generation]() -> AsyncBuildResult {
        AsyncBuildResult output;
        output.generation = generation;
        output.result = DisplayRasterizer::convert(source,
                                                   width,
                                                   height,
                                                   colorMode,
                                                   scaleMode,
                                                   filterParams,
                                                   encodingMode,
                                                   monoThreshold,
                                                   invertMono,
                                                   linearColorSpace);
        if (output.result.preview.isNull())
            return output;

        DisplayProfile profile = DisplayProfile::byId(profileId);
        profile.width = width;
        profile.height = height;
        profile.colorMode = colorMode;
        if (profile.id == QStringLiteral("custom"))
            profile.name = QCoreApplication::translate("PixelStudio", "Custom %1×%2")
                               .arg(width)
                               .arg(height);

        output.generatedCode = DisplayCodeGenerator::generate(profile,
                                                              width,
                                                              height,
                                                              encodingMode,
                                                              output.result.monoBits,
                                                              output.result.monoBuffer,
                                                              output.result.grayscale8,
                                                              output.result.rgb565,
                                                              output.result.rgb888,
                                                              output.result.rgb233,
                                                              output.result.rgb24,
                                                              arrayName,
                                                              monoLayout,
                                                              codeOptions,
                                                              output.result.indexedPalette);
        return output;
    });
    m_rebuildWatcher.setFuture(future);
}

void DisplayConverter::onAsyncRebuildFinished()
{
    const AsyncBuildResult result = m_rebuildWatcher.result();
    if (result.generation >= m_lastAppliedGeneration) {
        m_lastAppliedGeneration = result.generation;
        m_lastResult = result.result;

        if (m_lastResult.preview.isNull()) {
            m_previewPath.clear();
            m_processPreviewPath.clear();
            m_generatedCode.clear();
            emit previewPathChanged();
            emit processPreviewPathChanged();
            emit generatedCodeChanged();
            updateCodePreview();
            updateFlashReport();
        } else {
            m_processPreviewPath = writeTempPreview(QStringLiteral("process"), m_lastResult.processPreview);
            emit processPreviewPathChanged();
            m_previewPath = writeTempPreview(QStringLiteral("preview"), m_lastResult.preview);
            emit previewPathChanged();
            m_generatedCode = result.generatedCode;
            emit generatedCodeChanged();
            updateCodePreview();
            updateFlashReport();
        }
    }

    if (m_rebuildPending) {
        m_rebuildPending = false;
        startAsyncRebuild();
    }
}

void DisplayConverter::markOrientedDirty()
{
    m_orientedDirty = true;
    m_orientedCache = QImage();
}

QUrl DisplayConverter::writeTempPreview(const QString &slotName, const QImage &img)
{
    if (img.isNull() || slotName.isEmpty())
        return {};

    const QString dir = AppPaths::runtimePreviewsDir();
    QDir().mkpath(dir);
    const QString path = dir + QLatin1Char('/') + slotName + QStringLiteral(".png");
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return {};
    if (!img.save(&file, "PNG") || !file.commit())
        return {};

    // Fixed filenames are reused on disk; bump URL so QML Image reloads updated PNG.
    QUrl url = QUrl::fromLocalFile(path);
    url.setQuery(QString::number(++m_previewEpoch));
    return url;
}

ConvertPipelineParams DisplayConverter::pipelineParams() const
{
    ConvertPipelineParams p;
    p.displayWidth = m_displayWidth;
    p.displayHeight = m_displayHeight;
    p.colorMode = m_colorMode;
    p.scaleMode = m_scaleMode;
    p.monoThreshold = m_monoThreshold;
    p.invertMono = m_invertMono;
    p.rotation = m_rotation;
    p.flipHorizontal = m_flipHorizontal;
    p.flipVertical = m_flipVertical;
    p.filterParams = m_filterParams;
    p.encodingMode = m_encodingMode;
    p.linearColorSpace = m_linearColorSpace;
    return p;
}

SessionSnapshot DisplayConverter::sessionSnapshot() const
{
    SessionSnapshot s;
    s.profileId = m_profileId;
    s.displayWidth = m_displayWidth;
    s.displayHeight = m_displayHeight;
    s.scaleMode = static_cast<int>(m_scaleMode);
    s.dithering = m_dithering;
    s.monoThreshold = m_monoThreshold;
    s.arrayName = m_arrayName;
    s.encodingMode = static_cast<int>(m_encodingMode);
    s.monoLayout = static_cast<int>(m_monoLayout);
    s.rotation = m_rotation;
    s.flipHorizontal = m_flipHorizontal;
    s.flipVertical = m_flipVertical;
    s.invertMono = m_invertMono;
    s.showGrid = m_showGrid;
    s.gridThresholdZoom = m_gridThresholdZoom;
    s.filterParams = m_filterParams;
    s.codeIncludeComments = m_codeGenOptions.includeHeaderComments;
    s.codeUseProgmem = m_codeGenOptions.useProgmem;
    s.codeStaticStorage = m_codeGenOptions.staticStorage;
    s.rgb565BigEndian = m_codeGenOptions.rgb565BigEndian;
    s.codeDmaAlign = m_codeGenOptions.dmaPaddingAlign;
    s.linearColorSpace = m_linearColorSpace;
    return s;
}

void DisplayConverter::applySessionSnapshot(const SessionSnapshot &snapshot)
{
    m_profileId = snapshot.profileId;
    m_displayWidth = snapshot.displayWidth;
    m_displayHeight = snapshot.displayHeight;
    m_scaleMode = static_cast<DisplayProfile::ScaleMode>(qBound(0, snapshot.scaleMode, 2));
    m_dithering = snapshot.dithering;
    m_monoThreshold = snapshot.monoThreshold;
    m_arrayName = snapshot.arrayName;
    const int storedEncoding = snapshot.encodingMode;
    if (storedEncoding >= 0
        && storedEncoding < static_cast<int>(DisplayCodeGenerator::EncodingMode::Count)) {
        m_encodingMode = static_cast<DisplayCodeGenerator::EncodingMode>(storedEncoding);
    } else {
        m_encodingMode = PixelFormatCatalog::migrateLegacy(storedEncoding);
    }
    m_monoLayout = static_cast<DisplayCodeGenerator::MonoLayout>(qBound(0, snapshot.monoLayout, 2));
    m_rotation = snapshot.rotation;
    m_flipHorizontal = snapshot.flipHorizontal;
    m_flipVertical = snapshot.flipVertical;
    m_invertMono = snapshot.invertMono;
    m_showGrid = snapshot.showGrid;
    m_gridThresholdZoom = snapshot.gridThresholdZoom;
    m_filterParams = snapshot.filterParams;
    m_filterParams.threshold = m_monoThreshold;
    m_codeGenOptions.includeHeaderComments = snapshot.codeIncludeComments;
    m_codeGenOptions.useProgmem = snapshot.codeUseProgmem;
    m_codeGenOptions.staticStorage = snapshot.codeStaticStorage;
    m_codeGenOptions.rgb565BigEndian = snapshot.rgb565BigEndian;
    m_codeGenOptions.dmaPaddingAlign = qBound(0, snapshot.codeDmaAlign, 8);
    if (m_codeGenOptions.dmaPaddingAlign != 4 && m_codeGenOptions.dmaPaddingAlign != 8)
        m_codeGenOptions.dmaPaddingAlign = 0;
    m_linearColorSpace = snapshot.linearColorSpace;
    if (m_filterParams.ditherMode == ImageFiltersPipeline::DitherMode::FloydSteinberg)
        m_dithering = true;
    else if (m_filterParams.ditherMode == ImageFiltersPipeline::DitherMode::None)
        m_dithering = false;
    syncProfileIdFromDimensions();
    m_colorMode = encodingIsColorMode(static_cast<int>(m_encodingMode))
        ? DisplayProfile::Rgb565
        : DisplayProfile::Mono1Bit;
    emit codeGenOptionsChanged();
    emit rgb565BigEndianChanged();
    emit codeDmaAlignChanged();
    emit linearColorSpaceChanged();
    emit tonePresetChanged();
    emit profileIdChanged();
    emit displayWidthChanged();
    emit displayHeightChanged();
    emit scaleModeChanged();
    emit ditheringChanged();
    emit monoThresholdChanged();
    emit arrayNameChanged();
    emit encodingModeChanged();
    emit monoLayoutChanged();
    emit rotationChanged();
    emit flipHorizontalChanged();
    emit flipVerticalChanged();
    emit invertMonoChanged();
    emit filterInvertChanged();
    emit showGridChanged();
    emit gridThresholdZoomChanged();
    emit colorModeChanged();
    emitAllFilterSignals();
}

void DisplayConverter::schedulePersistSession()
{
    if (m_session)
        m_sessionSaveTimer.start();
}

void DisplayConverter::persistSession()
{
    persistUiState();
}

void DisplayConverter::persistUiState()
{
    if (!m_session)
        return;
    if (!m_projectFile.isEmpty())
        m_uiState.lastProjectFile = m_projectFile.toLocalFile();
    m_session->saveUiState(m_uiState);
    emit uiFoldersChanged();
}

QString DisplayConverter::lastOpenImageDir() const
{
    if (!m_uiState.lastOpenImageDir.isEmpty())
        return m_uiState.lastOpenImageDir;
    return AppPaths::userDocumentsRoot();
}

QString DisplayConverter::lastExportDir() const
{
    if (!m_uiState.lastExportDir.isEmpty())
        return m_uiState.lastExportDir;
    return AppPaths::exportsDir();
}

void DisplayConverter::applyStoredUiState()
{
    const QString watchOut = m_uiState.watchOutputFolder.isEmpty()
        ? AppPaths::watchDir()
        : m_uiState.watchOutputFolder;
    if (!m_uiState.watchInputFolder.isEmpty() || !watchOut.isEmpty())
        m_watchService.configure(m_uiState.watchInputFolder, watchOut);
    if (m_uiState.watchActive)
        m_watchService.setActive(true);

    emit uiFoldersChanged();
}

QString DisplayConverter::lastProjectPath() const
{
    if (AppPaths::isInternalDataPath(m_uiState.lastProjectFile))
        return {};
    return m_uiState.lastProjectFile;
}

bool DisplayConverter::hasRestorableProject() const
{
    const QString path = lastProjectPath();
    return !path.isEmpty() && QFileInfo::exists(path);
}

bool DisplayConverter::restoreLastProject()
{
    if (!m_appSettings || !m_appSettings->restoreLastProject())
        return false;
    const QString path = lastProjectPath();
    if (path.isEmpty() || !QFileInfo::exists(path))
        return false;
    return openProject(QUrl::fromLocalFile(path));
}

void DisplayConverter::openUserDocumentsFolder()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(AppPaths::userDocumentsRoot()));
}

QString DisplayConverter::suggestedCodeFilePath() const
{
    QString base = DisplayCodeGenerator::sanitizeIdentifier(m_arrayName);
    if (base.isEmpty())
        base = QStringLiteral("image_data");
    const QString dir = lastExportDir();
    return dir + QLatin1Char('/') + base + QStringLiteral(".h");
}

QUrl DisplayConverter::suggestedCodeFileUrl() const
{
    return QUrl::fromLocalFile(suggestedCodeFilePath());
}

void DisplayConverter::rememberOpenImageDir(const QString &dir)
{
    const QString path = QFileInfo(dir).isDir() ? dir : QFileInfo(dir).absolutePath();
    if (path.isEmpty())
        return;
    m_uiState.lastOpenImageDir = path;
    schedulePersistSession();
}

void DisplayConverter::rememberExportDir(const QString &dir)
{
    const QString path = QFileInfo(dir).isDir() ? dir : QFileInfo(dir).absolutePath();
    if (path.isEmpty())
        return;
    m_uiState.lastExportDir = path;
    schedulePersistSession();
}

void DisplayConverter::refreshLocalization()
{
    ++m_localizationRevision;
    emit localizationRevisionChanged();
    emit colorModeChanged();
    emit encodingModeChanged();
    emit monoLayoutChanged();
    emit profileIdChanged();
}

QVariantList DisplayConverter::recentFiles() const
{
    return m_session ? m_session->recentFiles() : QVariantList{};
}

QVariantList DisplayConverter::recentExports() const
{
    return m_session ? m_session->recentExports() : QVariantList{};
}

void DisplayConverter::flushPersistence()
{
    m_sessionSaveTimer.stop();
    persistSession();
}

void DisplayConverter::openAppDataFolder()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(AppPaths::dataRoot()));
}

void DisplayConverter::resetSession()
{
    if (!m_session)
        return;
    m_session->resetToDefaults();
    applySessionSnapshot(SessionSettings::defaultSnapshot());
    m_uiState = SessionUiState{};
    m_uiState.watchOutputFolder = AppPaths::watchDir();
    m_watchService.configure(QString(), m_uiState.watchOutputFolder);
    m_watchService.setActive(false);
    schedulePersistSession();
    emit recentFilesChanged();
    emit recentExportsChanged();
    emit uiFoldersChanged();
    emit watchFolderChanged();
}

bool DisplayConverter::exportSettingsTo(const QUrl &folderUrl)
{
    flushPersistence();
    QString dir = folderUrl.toLocalFile();
    if (dir.isEmpty())
        dir = folderUrl.path();
    if (dir.isEmpty())
        return false;
    QDir().mkpath(dir);
    const QString appIni = AppPaths::appSettingsFile();
    const QString sessionIni = AppPaths::sessionSettingsFile();
    bool ok = true;
    ok = QFile::copy(appIni, QDir(dir).absoluteFilePath(QStringLiteral("app.ini"))) && ok;
    ok = QFile::copy(sessionIni, QDir(dir).absoluteFilePath(QStringLiteral("session.ini"))) && ok;
    return ok;
}

bool DisplayConverter::importSettingsFrom(const QUrl &folderUrl)
{
    QString dir = folderUrl.toLocalFile();
    if (dir.isEmpty())
        dir = folderUrl.path();
    if (dir.isEmpty())
        return false;

    const QString srcApp = QDir(dir).absoluteFilePath(QStringLiteral("app.ini"));
    const QString srcSession = QDir(dir).absoluteFilePath(QStringLiteral("session.ini"));
    if (!QFileInfo::exists(srcApp) && !QFileInfo::exists(srcSession)) {
        emit errorOccurred(AppLocale::tr("No app.ini or session.ini in selected folder"));
        return false;
    }

    flushPersistence();
    bool ok = true;
    if (QFileInfo::exists(srcApp))
        ok = QFile::copy(srcApp, AppPaths::appSettingsFile()) && ok;
    if (QFileInfo::exists(srcSession))
        ok = QFile::copy(srcSession, AppPaths::sessionSettingsFile()) && ok;
    if (!ok) {
        emit errorOccurred(AppLocale::tr("Settings import failed"));
        return false;
    }
    return true;
}

void DisplayConverter::updateWatchExportPrefix()
{
    QString stem = DisplayCodeGenerator::sanitizeIdentifier(m_project.name);
    if (stem.isEmpty())
        stem = QStringLiteral("project");
    m_watchService.setExportNamePrefix(stem);
}

void DisplayConverter::restartAutosaveTimer()
{
    m_autosaveTimer.stop();
    if (!m_appSettings || !m_appSettings->projectAutosave())
        return;
    m_autosaveTimer.setInterval(qMax(30, m_appSettings->projectAutosaveSeconds()) * 1000);
    m_autosaveTimer.start();
}

void DisplayConverter::onAutosaveTimeout()
{
    if (!m_appSettings || !m_appSettings->projectAutosave())
        return;
    if (m_projectFile.isEmpty() || !hasImage())
        return;
    saveProject();
}

void DisplayConverter::setOffsetX(int value)
{
    if (m_offsetX == value)
        return;
    m_offsetX = value;
    emit offsetChanged();
    schedulePersistSession();
}

void DisplayConverter::setOffsetY(int value)
{
    if (m_offsetY == value)
        return;
    m_offsetY = value;
    emit offsetChanged();
    schedulePersistSession();
}

void DisplayConverter::setShowFullGeneratedCode(bool on)
{
    if (m_showFullGeneratedCode == on)
        return;
    m_showFullGeneratedCode = on;
    emit showFullGeneratedCodeChanged();
    updateCodePreview();
}

void DisplayConverter::updateCodePreview()
{
    if (m_showFullGeneratedCode || m_generatedCode.isEmpty()) {
        m_generatedCodePreview = m_generatedCode;
        m_generatedCodeTruncated = false;
    } else {
        constexpr int kMaxLines = 80;
        constexpr int kMaxChars = 12000;
        QStringList lines = m_generatedCode.split(QLatin1Char('\n'));
        m_generatedCodeTruncated = lines.size() > kMaxLines || m_generatedCode.size() > kMaxChars;
        if (lines.size() > kMaxLines)
            lines = lines.mid(0, kMaxLines);
        m_generatedCodePreview = lines.join(QLatin1Char('\n'));
        if (m_generatedCodePreview.size() > kMaxChars)
            m_generatedCodePreview = m_generatedCodePreview.left(kMaxChars);
        if (m_generatedCodeTruncated)
            m_generatedCodePreview += QStringLiteral("\n\n// … %1 bytes omitted — use Copy for full output\n")
                                          .arg(m_generatedCode.size());
    }
    emit generatedCodePreviewChanged();
}

void DisplayConverter::updateFlashReport()
{
    m_flashReport = EncodingAnalyzer::analyze(m_lastResult,
                                              m_colorMode,
                                              m_monoLayout,
                                              m_encodingMode,
                                              m_codeGenOptions);
    emit flashReportChanged();
}

StudioProject DisplayConverter::projectSnapshot() const
{
    StudioProject project = ProjectService::fromSession(m_project.name, sessionSnapshot(), m_offsetX, m_offsetY);
    project.assets.clear();
    project.sourceImagePng.clear();
    project.resultPreviewPng.clear();

    auto savePng = [](const QImage &image) -> QByteArray {
        if (image.isNull())
            return {};
        QByteArray png;
        QBuffer buffer(&png);
        buffer.open(QIODevice::WriteOnly);
        return image.save(&buffer, "PNG") ? png : QByteArray{};
    };

    const bool hasSourceFile = !m_sourceFilePath.isEmpty() && QFileInfo::exists(m_sourceFilePath);
    if (hasSourceFile) {
        project.assets.append(ProjectAsset{
            m_sourceFilePath,
            QFileInfo(m_sourceFilePath).fileName(),
            0,
            0,
        });
    } else if (!m_sourceImage.isNull()) {
        project.sourceImagePng = savePng(m_sourceImage);
    }

    project.resultPreviewPng = savePng(m_lastResult.preview);
    return project;
}

void DisplayConverter::applyProject(const StudioProject &project)
{
    m_project = project;
    m_offsetX = project.offsetX;
    m_offsetY = project.offsetY;
    m_sourceFilePath.clear();
    applySessionSnapshot(project.session);

    QImage image;
    for (const ProjectAsset &asset : project.assets) {
        if (!QFileInfo::exists(asset.path))
            continue;
        if (m_loader->loadFromFile(QUrl::fromLocalFile(asset.path), image)) {
            m_sourceFilePath = QFileInfo(asset.path).absoluteFilePath();
            break;
        }
    }
    if (image.isNull() && !project.sourceImagePng.isEmpty())
        image.loadFromData(project.sourceImagePng, "PNG");

    if (!image.isNull()) {
        m_sourceImage = image;
        markOrientedDirty();
        refreshSourcePreview();
        emit hasImageChanged();
        emit sourceWidthChanged();
        emit sourceHeightChanged();
        scheduleRebuild(true);
    } else {
        m_sourceImage = QImage();
        m_sourcePath.clear();
        emit hasImageChanged();
        emit sourcePathChanged();
        scheduleRebuild(true);
    }

    emit offsetChanged();
    updateWatchExportPrefix();
    emit projectChanged();
}

void DisplayConverter::onWatchExportRequested(const QVariantList &files, const QUrl &targetFile)
{
    enqueueBatchCodeExport(files, targetFile);
}
