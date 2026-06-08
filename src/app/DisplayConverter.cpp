#include "app/DisplayConverter.h"
#include "app/PreviewImageProvider.h"
#include "app/converter/ExportController.h"
#include "app/converter/ImagePipelineController.h"
#include "app/converter/ProjectSessionController.h"
#include "translation/AppLocale.h"
#include "persistence/AppPaths.h"
#include "persistence/AppSettings.h"
#include "processing/DisplayCodeGenerator.h"
#include "processing/PixelFormatCatalog.h"
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
    connect(m_loader, &ImageLoader::loadingChanged, this, [this]() {
        const bool loading = m_loader->loading();
        if (m_state.imageLoading == loading)
            return;
        m_state.imageLoading = loading;
        emit imageLoadingChanged();
    });
    m_state.filterParams.ditherMode = ImageFiltersPipeline::DitherMode::FloydSteinberg;
    m_state.filterParams.threshold = m_state.monoThreshold;
    m_rebuildDebounceTimer.setSingleShot(true);
    m_rebuildDebounceTimer.setInterval(200);
    connect(&m_rebuildDebounceTimer, &QTimer::timeout, this, [this]() {
        ImagePipelineController::startAsyncRebuild(*this);
    });
    connect(&m_rebuildWatcher, &QFutureWatcher<ConverterAsyncBuildResult>::finished,
            this, [this]() { ImagePipelineController::onAsyncRebuildFinished(*this); });
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
        m_session->loadUiState(&m_state.uiState);
        m_session->pruneMissingRecentFiles();
        m_session->pruneMissingRecentExports();
        applyStoredUiState();
        updateWatchExportPrefix();
        restartAutosaveTimer();
    } else {
        applyProfile(DisplayProfile::byId(m_state.profileId));
    }
}

void DisplayConverter::setDisplayWidth(int w)
{
    w = qBound(8, w, 1024);
    if (m_state.displayWidth == w)
        return;
    m_state.displayWidth = w;
    emit displayWidthChanged();
    syncProfileIdFromDimensions();
    scheduleRebuild();
}

void DisplayConverter::setDisplayHeight(int h)
{
    h = qBound(8, h, 1024);
    if (m_state.displayHeight == h)
        return;
    m_state.displayHeight = h;
    emit displayHeightChanged();
    syncProfileIdFromDimensions();
    scheduleRebuild();
}

void DisplayConverter::setProfileId(const QString &id)
{
    if (m_state.profileId == id)
        return;
    m_state.profileId = id;
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
    if (m_state.colorMode == next)
        return;
    m_state.colorMode = next;

    const int enc = static_cast<int>(m_state.encodingMode);
    if (m_state.colorMode == DisplayProfile::Rgb565) {
        if (!encodingIsColorMode(enc))
            m_state.encodingMode = DisplayCodeGenerator::EncodingMode::Rgb565;
    } else {
        if (!encodingIsMonoMode(enc))
            m_state.encodingMode = DisplayCodeGenerator::EncodingMode::Mono1Bit;
    }

    if (m_state.colorMode != DisplayProfile::Mono1Bit
        && m_state.monoLayout != DisplayCodeGenerator::MonoLayout::RowPacked) {
        m_state.monoLayout = DisplayCodeGenerator::MonoLayout::RowPacked;
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
    if (m_state.scaleMode == m)
        return;
    m_state.scaleMode = m;
    emit scaleModeChanged();
    scheduleRebuild();
}

void DisplayConverter::setDithering(bool on)
{
    const auto mode = on
        ? ImageFiltersPipeline::DitherMode::FloydSteinberg
        : ImageFiltersPipeline::DitherMode::None;
    if (m_state.dithering == on && m_state.filterParams.ditherMode == mode)
        return;
    m_state.dithering = on;
    m_state.filterParams.ditherMode = mode;
    emit ditheringChanged();
    emit ditherModeChanged();
    scheduleRebuild();
}

void DisplayConverter::setArrayName(const QString &name)
{
    const QString safe = DisplayCodeGenerator::sanitizeIdentifier(name);
    if (m_state.arrayName == safe)
        return;
    m_state.arrayName = safe;
    emit arrayNameChanged();
    schedulePersistSession();
    if (hasImage())
        scheduleRebuild();
}

int DisplayConverter::dataByteCount() const
{
    return DisplayCodeGenerator::flashFootprintBytes(m_state.encodingMode,
                                                     m_state.displayWidth,
                                                     m_state.displayHeight,
                                                     m_state.lastResult.monoBits,
                                                     m_state.lastResult.monoBuffer,
                                                     m_state.lastResult.grayscale8,
                                                     m_state.lastResult.rgb565,
                                                     m_state.lastResult.rgb888,
                                                     m_state.lastResult.rgb233,
                                                     m_state.lastResult.rgb24,
                                                     m_state.monoLayout,
                                                     m_state.codeGenOptions,
                                                     m_state.lastResult.indexedPalette);
}

QString DisplayConverter::colorModeName() const
{
    return m_state.colorMode == DisplayProfile::Rgb565
        ? AppLocale::tr("Color")
        : AppLocale::tr("B&W");
}

QString DisplayConverter::encodingModeName() const
{
    const QVariantList modes = availableEncodingModes();
    for (const QVariant &v : modes) {
        const QVariantMap m = v.toMap();
        if (m.value(QStringLiteral("mode")).toInt() == static_cast<int>(m_state.encodingMode))
            return m.value(QStringLiteral("name")).toString();
    }
    return AppLocale::tr("Unknown");
}

bool DisplayConverter::encodingIsMono1Bit() const
{
    return encodingIsMonoMode(static_cast<int>(m_state.encodingMode));
}

bool DisplayConverter::encodingIsGrayscale() const
{
    return PixelFormatCatalog::isGrayscale(m_state.encodingMode);
}

bool DisplayConverter::encodingIsColor() const
{
    return encodingIsColorMode(static_cast<int>(m_state.encodingMode));
}

QString DisplayConverter::monoLayoutName() const
{
    using Layout = DisplayCodeGenerator::MonoLayout;
    if (m_state.monoLayout == Layout::Ssd1306Page)
        return AppLocale::tr("Vertical page buffer");
    if (m_state.monoLayout == Layout::VerticalColumn)
        return AppLocale::tr("Vertical column");
    return AppLocale::tr("Row-packed");
}

QVariantList DisplayConverter::previewPalette() const
{
    if (!m_state.lastResult.indexedPalette.isEmpty())
        return paletteFromRgbList(m_state.lastResult.indexedPalette);
    if (!m_state.lastResult.preview.isNull())
        return paletteFromPreviewImage(m_state.lastResult.preview);
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
    const QString path = m_state.sourceFilePath;
    const int dot = path.lastIndexOf(QLatin1Char('.'));
    if (dot >= 0 && dot < path.size() - 1)
        return path.mid(dot + 1).toUpper();
    return AppLocale::tr("Image");
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
    if (m_state.rotation == degrees)
        return;
    m_state.rotation = degrees;
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
    setRotation((m_state.rotation + 90) % 360);
}

QImage DisplayConverter::orientedSource() const
{
    if (m_state.sourceImage.isNull())
        return {};
    if (!m_state.orientedDirty && !m_state.orientedCache.isNull())
        return m_state.orientedCache;

    m_state.orientedCache = ConvertPipeline::applyOrientation(m_state.sourceImage, pipelineParams());
    m_state.orientedDirty = false;
    return m_state.orientedCache;
}

void DisplayConverter::setFlipHorizontal(bool on)
{
    if (m_state.flipHorizontal == on)
        return;
    m_state.flipHorizontal = on;
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
    if (m_state.flipVertical == on)
        return;
    m_state.flipVertical = on;
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
    if (m_state.invertMono == on)
        return;
    m_state.invertMono = on;
    emit invertMonoChanged();
    if (hasImage())
        scheduleRebuild();
}

void DisplayConverter::setFilterInvert(bool on)
{
    if (m_state.filterParams.invert == on)
        return;
    m_state.filterParams.invert = on;
    emit filterInvertChanged();
    if (hasImage())
        scheduleRebuild();
}

void DisplayConverter::setMonoLayout(int mode)
{
    const auto next = static_cast<DisplayCodeGenerator::MonoLayout>(qBound(0, mode, 2));
    if (m_state.monoLayout == next)
        return;
    m_state.monoLayout = next;
    emit monoLayoutChanged();
    if (hasImage() && encodingIsMono1Bit())
        scheduleRebuild();
}

void DisplayConverter::setEncodingMode(int mode)
{
    const auto next = static_cast<DisplayCodeGenerator::EncodingMode>(
        qBound(0, mode, static_cast<int>(DisplayCodeGenerator::EncodingMode::Count) - 1));
    if (m_state.encodingMode == next)
        return;
    m_state.encodingMode = next;

    const auto nextColor = encodingIsColorMode(static_cast<int>(next))
                               ? DisplayProfile::Rgb565
                               : DisplayProfile::Mono1Bit;
    if (m_state.colorMode != nextColor) {
        m_state.colorMode = nextColor;
        emit colorModeChanged();
    }
    if (m_state.colorMode != DisplayProfile::Mono1Bit
        && m_state.monoLayout != DisplayCodeGenerator::MonoLayout::RowPacked) {
        m_state.monoLayout = DisplayCodeGenerator::MonoLayout::RowPacked;
        emit monoLayoutChanged();
    }

    emit encodingModeChanged();
    if (hasImage())
        scheduleRebuild();
}

void DisplayConverter::swapDisplayDimensions()
{
    const int w = m_state.displayWidth;
    const int h = m_state.displayHeight;
    if (w == h)
        return;
    m_state.displayWidth = h;
    m_state.displayHeight = w;
    emit displayWidthChanged();
    emit displayHeightChanged();
    syncProfileIdFromDimensions();
    scheduleRebuild();
}

void DisplayConverter::refreshSourcePreview()
{
    if (m_state.sourceImage.isNull())
        return;
    m_state.sourcePath = publishPreview(QStringLiteral("source"), orientedSource());
    emit sourcePathChanged();
}

void DisplayConverter::setMonoThreshold(int value)
{
    value = qBound(0, value, 255);
    if (m_state.monoThreshold == value)
        return;
    m_state.monoThreshold = value;
    m_state.filterParams.threshold = value;
    emit monoThresholdChanged();
    if (hasImage() && encodingIsMono1Bit())
        scheduleRebuild();
}

void DisplayConverter::setBlackBackground(bool on)
{
    if (m_state.filterParams.blackBackground == on)
        return;
    m_state.filterParams.blackBackground = on;
    emit blackBackgroundChanged();
    scheduleRebuild();
}

void DisplayConverter::setBrightness(int value)
{
    value = qBound(0, value, 200);
    if (m_state.filterParams.brightness == value)
        return;
    m_state.filterParams.brightness = value;
    markToneCustom();
    emit brightnessChanged();
    scheduleRebuild();
}

void DisplayConverter::setContrast(int value)
{
    value = qBound(0, value, 200);
    if (m_state.filterParams.contrast == value)
        return;
    m_state.filterParams.contrast = value;
    markToneCustom();
    emit contrastChanged();
    scheduleRebuild();
}

void DisplayConverter::setSaturation(int value)
{
    value = qBound(0, value, 200);
    if (m_state.filterParams.saturation == value)
        return;
    m_state.filterParams.saturation = value;
    markToneCustom();
    emit saturationChanged();
    scheduleRebuild();
}

void DisplayConverter::setExposure(int value)
{
    value = qBound(50, value, 200);
    if (m_state.filterParams.exposure == value)
        return;
    m_state.filterParams.exposure = value;
    markToneCustom();
    emit exposureChanged();
    scheduleRebuild();
}

void DisplayConverter::setGamma(int value)
{
    value = qBound(50, value, 200);
    if (m_state.filterParams.gamma == value)
        return;
    m_state.filterParams.gamma = value;
    markToneCustom();
    emit gammaChanged();
    scheduleRebuild();
}

void DisplayConverter::setBlur(int value)
{
    value = qBound(0, value, 6);
    if (m_state.filterParams.blur == value)
        return;
    m_state.filterParams.blur = value;
    emit blurChanged();
    scheduleRebuild();
}

void DisplayConverter::setPosterizeRgb(int value)
{
    value = qBound(0, value, 30);
    if (m_state.filterParams.posterizeRgb == value)
        return;
    m_state.filterParams.posterizeRgb = value;
    emit posterizeRgbChanged();
    scheduleRebuild();
}

void DisplayConverter::setColorMaskEnabled(bool on)
{
    if (m_state.filterParams.colorMaskEnabled == on)
        return;
    m_state.filterParams.colorMaskEnabled = on;
    emit colorMaskEnabledChanged();
    scheduleRebuild();
}

void DisplayConverter::setMaskColor(const QColor &color)
{
    const QColor safe = color.isValid() ? color : QColor(Qt::black);
    if (m_state.filterParams.maskColor == safe)
        return;
    m_state.filterParams.maskColor = safe;
    emit maskColorChanged();
    scheduleRebuild();
}

void DisplayConverter::setMaskTolerance(int value)
{
    value = qBound(0, value, 255);
    if (m_state.filterParams.maskTolerance == value)
        return;
    m_state.filterParams.maskTolerance = value;
    emit maskToleranceChanged();
    scheduleRebuild();
}

void DisplayConverter::setMaskAmplify(int value)
{
    value = qBound(1, value, 10);
    if (m_state.filterParams.maskAmplify == value)
        return;
    m_state.filterParams.maskAmplify = value;
    emit maskAmplifyChanged();
    scheduleRebuild();
}

void DisplayConverter::setSharpen(bool on)
{
    if (m_state.filterParams.sharpen == on)
        return;
    m_state.filterParams.sharpen = on;
    emit sharpenChanged();
    scheduleRebuild();
}

void DisplayConverter::setSobelEdges(int value)
{
    value = qBound(0, value, 100);
    if (m_state.filterParams.sobelEdges == value)
        return;
    m_state.filterParams.sobelEdges = value;
    emit sobelEdgesChanged();
    scheduleRebuild();
}

void DisplayConverter::setPosterizeGray(int value)
{
    value = qBound(0, value, 30);
    if (m_state.filterParams.posterizeGray == value)
        return;
    m_state.filterParams.posterizeGray = value;
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
    if (m_state.filterParams.ditherMode == next)
        return;
    m_state.filterParams.ditherMode = next;
    m_state.dithering = (next == ImageFiltersPipeline::DitherMode::FloydSteinberg);
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
    if (m_state.filterParams.contourMode == next)
        return;
    m_state.filterParams.contourMode = next;
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
    if (m_state.filterParams.tonePreset == ImageFiltersPipeline::TonePreset::Custom)
        return;
    m_state.filterParams.tonePreset = ImageFiltersPipeline::TonePreset::Custom;
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
    return m_state.codeGenOptions;
}

void DisplayConverter::setTonePreset(int preset)
{
    const auto next = static_cast<ImageFiltersPipeline::TonePreset>(qBound(0, preset, 2));
    const ImageFiltersPipeline::Params tone = ImageFiltersPipeline::paramsForTonePreset(next);
    m_state.filterParams.tonePreset = next;
    if (next != ImageFiltersPipeline::TonePreset::Custom) {
        m_state.filterParams.brightness = tone.brightness;
        m_state.filterParams.contrast = tone.contrast;
        m_state.filterParams.saturation = tone.saturation;
        m_state.filterParams.exposure = tone.exposure;
        m_state.filterParams.gamma = tone.gamma;
        m_state.filterParams.posterizeGray = tone.posterizeGray;
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
    const int threshold = m_state.monoThreshold;
    const bool invert = m_state.invertMono;
    m_state.filterParams = ImageFiltersPipeline::Params{};
    m_state.filterParams.threshold = threshold;
    m_state.dithering = true;
    m_state.filterParams.ditherMode = ImageFiltersPipeline::DitherMode::FloydSteinberg;
    m_state.invertMono = invert;
    m_state.filterParams.invert = false;
    emit ditheringChanged();
    emit invertMonoChanged();
    emitAllFilterSignals();
    scheduleRebuild();
}

void DisplayConverter::resetTransform()
{
    m_state.rotation = 0;
    m_state.flipHorizontal = false;
    m_state.flipVertical = false;
    m_state.scaleMode = DisplayProfile::Fit;
    m_state.offsetX = 0;
    m_state.offsetY = 0;
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
                                            m_state.displayWidth,
                                            m_state.displayHeight,
                                            m_state.scaleMode);
    if (content.isEmpty())
        return;
    setOffsetX((m_state.displayWidth - content.width()) / 2);
    setOffsetY((m_state.displayHeight - content.height()) / 2);
}

void DisplayConverter::copyGeneratedArray()
{
    copyToClipboard(DisplayCodeGenerator::extractArrayBody(m_state.generatedCode));
}

void DisplayConverter::setCodeIncludeComments(bool on)
{
    if (m_state.codeGenOptions.includeHeaderComments == on)
        return;
    m_state.codeGenOptions.includeHeaderComments = on;
    emit codeGenOptionsChanged();
    scheduleRebuild(true);
}

void DisplayConverter::setCodeUseProgmem(bool on)
{
    if (m_state.codeGenOptions.useProgmem == on)
        return;
    m_state.codeGenOptions.useProgmem = on;
    emit codeGenOptionsChanged();
    scheduleRebuild(true);
}

void DisplayConverter::setCodeStaticStorage(bool on)
{
    if (m_state.codeGenOptions.staticStorage == on)
        return;
    m_state.codeGenOptions.staticStorage = on;
    emit codeGenOptionsChanged();
    scheduleRebuild(true);
}

void DisplayConverter::setRgb565BigEndian(bool on)
{
    if (m_state.codeGenOptions.rgb565BigEndian == on)
        return;
    m_state.codeGenOptions.rgb565BigEndian = on;
    emit rgb565BigEndianChanged();
    emit codeGenOptionsChanged();
    scheduleRebuild(true);
}

void DisplayConverter::setCodeDmaAlign(int align)
{
    align = align == 8 ? 8 : (align == 4 ? 4 : 0);
    if (m_state.codeGenOptions.dmaPaddingAlign == align)
        return;
    m_state.codeGenOptions.dmaPaddingAlign = align;
    emit codeDmaAlignChanged();
    emit codeGenOptionsChanged();
    scheduleRebuild(true);
}

void DisplayConverter::setLinearColorSpace(bool on)
{
    if (m_state.linearColorSpace == on)
        return;
    m_state.linearColorSpace = on;
    emit linearColorSpaceChanged();
    if (hasImage())
        scheduleRebuild();
}

void DisplayConverter::setShowGrid(bool on)
{
    if (m_state.showGrid == on)
        return;
    m_state.showGrid = on;
    emit showGridChanged();
}

void DisplayConverter::setGridThresholdZoom(int value)
{
    value = qBound(1, value, 64);
    if (m_state.gridThresholdZoom == value)
        return;
    m_state.gridThresholdZoom = value;
    emit gridThresholdZoomChanged();
}

void DisplayConverter::syncProfileIdFromDimensions()
{
    for (const DisplayProfile &p : DisplayProfile::presets()) {
        if (p.id == QStringLiteral("custom"))
            continue;
        if (p.width == m_state.displayWidth && p.height == m_state.displayHeight) {
            if (m_state.profileId != p.id) {
                m_state.profileId = p.id;
                emit profileIdChanged();
            }
            return;
        }
    }
    if (m_state.profileId != QStringLiteral("custom")) {
        m_state.profileId = QStringLiteral("custom");
        emit profileIdChanged();
    }
}

void DisplayConverter::applyProfile(const DisplayProfile &profile)
{
    if (profile.id == QStringLiteral("custom"))
        return;

    m_state.displayWidth = profile.width;
    m_state.displayHeight = profile.height;
    emit displayWidthChanged();
    emit displayHeightChanged();
}

bool DisplayConverter::loadImage(const QUrl &url)
{
    if (url.scheme().startsWith(QStringLiteral("http"), Qt::CaseInsensitive)) {
        loadFromUrl(url.toString());
        return true;
    }
    m_loader->loadFromFileAsync(url);
    return true;
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
    m_state.rebuildPending = false;
    m_state.lastAppliedGeneration = ++m_state.nextGeneration;
    m_batchService.reset();
    m_state.sourceImage = QImage();
    m_state.sourceFilePath.clear();
    m_state.sourcePath.clear();
    m_state.previewPath.clear();
    m_state.processPreviewPath.clear();
    m_state.generatedCode.clear();
    m_state.flashReport.clear();
    m_state.lastResult = {};
    m_state.rotation = 0;
    m_state.flipHorizontal = false;
    m_state.flipVertical = false;
    m_state.invertMono = false;
    m_state.filterParams = ImageFiltersPipeline::Params{};
    m_state.filterParams.threshold = m_state.monoThreshold;
    m_state.filterParams.ditherMode = m_state.dithering
        ? ImageFiltersPipeline::DitherMode::FloydSteinberg
        : ImageFiltersPipeline::DitherMode::None;
    markOrientedDirty();
    if (m_previewProvider)
        m_previewProvider->clearAll();
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
    ImagePipelineController::updateCodePreview(m_state, *this);
}

QVariantList DisplayConverter::displayPresets() const
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

void DisplayConverter::newProject(const QString &name)
{
    ProjectSessionController::newProject(*this, name);
}

bool DisplayConverter::openProject(const QUrl &url)
{
    return ProjectSessionController::openProject(*this, url);
}

bool DisplayConverter::saveProject()
{
    return ProjectSessionController::saveProject(*this);
}

bool DisplayConverter::saveProjectAs(const QUrl &url)
{
    return ProjectSessionController::saveProjectAs(*this, url);
}

bool DisplayConverter::importHeader(const QUrl &url)
{
    const HeaderImportResult result = HeaderImportService::importHeader(url);
    if (!result.ok) {
        emit errorOccurred(result.errorMessage);
        return false;
    }
    m_state.sourceImage = result.preview;
    m_state.arrayName = result.arrayName;
    m_state.displayWidth = result.width;
    m_state.displayHeight = result.height;
    m_state.colorMode = result.colorMode;
    m_state.encodingMode = result.encodingMode;
    m_state.profileId = QStringLiteral("custom");
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
    ExportController::configureWatchFolder(*this, inputFolder, outputFolder);
}

void DisplayConverter::configureWatchFolders(const QUrl &inputFolder, const QUrl &outputFolder)
{
    configureWatchFolder(inputFolder.toLocalFile(), outputFolder.toLocalFile());
}

void DisplayConverter::setWatchFolderActive(bool active)
{
    ExportController::setWatchFolderActive(*this, active);
}

bool DisplayConverter::buildSpriteAtlas(const QVariantList &urls, const QUrl &targetFile, int frameWidth, int frameHeight)
{
    return ExportController::buildSpriteAtlas(*this, urls, targetFile, frameWidth, frameHeight);
}

void DisplayConverter::copyToClipboard(const QString &text)
{
    QGuiApplication::clipboard()->setText(text);
}

bool DisplayConverter::saveCodeToFile(const QUrl &url)
{
    return ExportController::saveCodeToFile(*this, url);
}

bool DisplayConverter::saveBinaryToFile(const QUrl &url)
{
    return ExportController::saveBinaryToFile(*this, url);
}

void DisplayConverter::enqueueBatchCodeExport(const QVariantList &urls, const QUrl &targetFile)
{
    ExportController::enqueueBatchCodeExport(*this, urls, targetFile);
}

void DisplayConverter::cancelBatchExport()
{
    m_batchService.cancel();
}

void DisplayConverter::onImageLoaded(const QImage &image, const QUrl &sourceUrl)
{
    m_state.sourceImage = image;
    m_state.rotation = 0;
    m_state.flipHorizontal = false;
    m_state.flipVertical = false;
    markOrientedDirty();
    if (sourceUrl.isLocalFile()) {
        const QString local = sourceUrl.toLocalFile();
        m_state.sourceFilePath = local.isEmpty() ? sourceUrl.path() : local;
    } else {
        m_state.sourceFilePath.clear();
    }
    refreshSourcePreview();
    emit hasImageChanged();
    emit rotationChanged();
    emit flipHorizontalChanged();
    emit flipVerticalChanged();
    emit sourceWidthChanged();
    emit sourceHeightChanged();
    if (m_session && sourceUrl.isLocalFile())
        m_session->addRecentFile(sourceUrl);
    if (sourceUrl.isLocalFile())
        rememberOpenImageDir(sourceUrl.toLocalFile().isEmpty() ? sourceUrl.path() : sourceUrl.toLocalFile());
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
    ImagePipelineController::scheduleRebuild(*this, immediate);
}

void DisplayConverter::setPreviewProvider(PreviewImageProvider *provider)
{
    m_previewProvider = provider;
}

void DisplayConverter::applyPipelineResult(const ConverterAsyncBuildResult &result)
{
    if (result.generation < m_state.lastAppliedGeneration)
        return;

    m_state.lastAppliedGeneration = result.generation;
    m_state.lastResult = result.result;
    if (m_state.lastResult.preview.isNull()) {
        m_state.previewPath.clear();
        m_state.processPreviewPath.clear();
        m_state.generatedCode.clear();
        emit previewPathChanged();
        emit processPreviewPathChanged();
        emit generatedCodeChanged();
        ImagePipelineController::updateCodePreview(m_state, *this);
        ImagePipelineController::updateFlashReport(m_state, *this);
        return;
    }

    m_state.processPreviewPath = publishPreview(QStringLiteral("process"), m_state.lastResult.processPreview);
    emit processPreviewPathChanged();
    m_state.previewPath = publishPreview(QStringLiteral("preview"), m_state.lastResult.preview);
    emit previewPathChanged();
    m_state.generatedCode = result.generatedCode;
    emit generatedCodeChanged();
    ImagePipelineController::updateCodePreview(m_state, *this);
    ImagePipelineController::updateFlashReport(m_state, *this);
}

StudioProject DisplayConverter::projectSnapshot() const
{
    return captureTabState().project;
}

void DisplayConverter::applyProject(const StudioProject &project)
{
    ProjectSessionController::applyProject(*this, project);
}

void DisplayConverter::markOrientedDirty()
{
    m_state.orientedDirty = true;
    m_state.orientedCache = QImage();
}

QUrl DisplayConverter::publishPreview(const QString &slotName, const QImage &img)
{
    if (img.isNull() || slotName.isEmpty() || !m_previewProvider)
        return {};
    m_previewProvider->setImage(slotName, img);
    return m_previewProvider->imageUrl(slotName);
}

ConverterTabSnapshot DisplayConverter::captureTabState() const
{
    ConverterTabSnapshot snapshot;
    snapshot.project = ProjectService::fromSession(m_state.project.name,
                                                     sessionSnapshot(),
                                                     m_state.offsetX,
                                                     m_state.offsetY);
    snapshot.project.assets.clear();
    snapshot.project.sourceImagePng.clear();
    snapshot.project.resultPreviewPng.clear();

    const bool hasSourceFile = !m_state.sourceFilePath.isEmpty()
        && QFileInfo::exists(m_state.sourceFilePath);
    if (hasSourceFile) {
        snapshot.project.assets.append(ProjectAsset{
            m_state.sourceFilePath,
            QFileInfo(m_state.sourceFilePath).fileName(),
            0,
            0,
        });
    }

    snapshot.sourceImage = m_state.sourceImage;
    snapshot.sourceFilePath = m_state.sourceFilePath;
    snapshot.projectFileUrl = m_state.projectFile;
    snapshot.lastResult = m_state.lastResult;
    snapshot.generatedCode = m_state.generatedCode;
    snapshot.hasPipelineResult = !m_state.lastResult.preview.isNull();
    return snapshot;
}

void DisplayConverter::restoreTabState(const ConverterTabSnapshot &snapshot)
{
    m_rebuildDebounceTimer.stop();
    m_state.rebuildPending = false;

    m_state.project = snapshot.project;
    m_state.offsetX = snapshot.project.offsetX;
    m_state.offsetY = snapshot.project.offsetY;
    m_state.sourceFilePath = snapshot.sourceFilePath;
    m_state.projectFile = snapshot.projectFileUrl;
    applySessionSnapshot(snapshot.project.session);

    m_state.sourceImage = snapshot.sourceImage;
    if (m_state.sourceImage.isNull() && !snapshot.project.sourceImagePng.isEmpty())
        m_state.sourceImage.loadFromData(snapshot.project.sourceImagePng, "PNG");

    if (!m_state.sourceImage.isNull()) {
        markOrientedDirty();
        refreshSourcePreview();
        emit hasImageChanged();
        emit sourceWidthChanged();
        emit sourceHeightChanged();
    } else {
        m_state.sourcePath.clear();
        if (m_previewProvider)
            m_previewProvider->clearSlot(QStringLiteral("source"));
        emit hasImageChanged();
        emit sourcePathChanged();
        emit sourceWidthChanged();
        emit sourceHeightChanged();
    }

    if (snapshot.hasPipelineResult) {
        m_state.lastResult = snapshot.lastResult;
        m_state.generatedCode = snapshot.generatedCode;
        m_state.processPreviewPath = publishPreview(QStringLiteral("process"), m_state.lastResult.processPreview);
        m_state.previewPath = publishPreview(QStringLiteral("preview"), m_state.lastResult.preview);
        emit processPreviewPathChanged();
        emit previewPathChanged();
        emit generatedCodeChanged();
        ImagePipelineController::updateCodePreview(m_state, *this);
        ImagePipelineController::updateFlashReport(m_state, *this);
    } else if (!m_state.sourceImage.isNull()) {
        ImagePipelineController::scheduleRebuild(*this, true);
    } else {
        m_state.previewPath.clear();
        m_state.processPreviewPath.clear();
        m_state.generatedCode.clear();
        m_state.lastResult = {};
        if (m_previewProvider) {
            m_previewProvider->clearSlot(QStringLiteral("preview"));
            m_previewProvider->clearSlot(QStringLiteral("process"));
        }
        emit previewPathChanged();
        emit processPreviewPathChanged();
        emit generatedCodeChanged();
        ImagePipelineController::updateCodePreview(m_state, *this);
        ImagePipelineController::updateFlashReport(m_state, *this);
    }

    emit offsetChanged();
    updateWatchExportPrefix();
    emit projectChanged();
}

StudioProject DisplayConverter::projectSnapshotForDisk() const
{
    return ProjectSessionController::projectSnapshotForDisk(*this);
}

ConvertPipelineParams DisplayConverter::pipelineParams() const
{
    ConvertPipelineParams p;
    p.displayWidth = m_state.displayWidth;
    p.displayHeight = m_state.displayHeight;
    p.colorMode = m_state.colorMode;
    p.scaleMode = m_state.scaleMode;
    p.monoThreshold = m_state.monoThreshold;
    p.invertMono = m_state.invertMono;
    p.rotation = m_state.rotation;
    p.flipHorizontal = m_state.flipHorizontal;
    p.flipVertical = m_state.flipVertical;
    p.filterParams = m_state.filterParams;
    p.encodingMode = m_state.encodingMode;
    p.linearColorSpace = m_state.linearColorSpace;
    return p;
}

SessionSnapshot DisplayConverter::sessionSnapshot() const
{
    SessionSnapshot s;
    s.profileId = m_state.profileId;
    s.displayWidth = m_state.displayWidth;
    s.displayHeight = m_state.displayHeight;
    s.scaleMode = static_cast<int>(m_state.scaleMode);
    s.dithering = m_state.dithering;
    s.monoThreshold = m_state.monoThreshold;
    s.arrayName = m_state.arrayName;
    s.encodingMode = static_cast<int>(m_state.encodingMode);
    s.monoLayout = static_cast<int>(m_state.monoLayout);
    s.rotation = m_state.rotation;
    s.flipHorizontal = m_state.flipHorizontal;
    s.flipVertical = m_state.flipVertical;
    s.invertMono = m_state.invertMono;
    s.showGrid = m_state.showGrid;
    s.gridThresholdZoom = m_state.gridThresholdZoom;
    s.filterParams = m_state.filterParams;
    s.codeIncludeComments = m_state.codeGenOptions.includeHeaderComments;
    s.codeUseProgmem = m_state.codeGenOptions.useProgmem;
    s.codeStaticStorage = m_state.codeGenOptions.staticStorage;
    s.rgb565BigEndian = m_state.codeGenOptions.rgb565BigEndian;
    s.codeDmaAlign = m_state.codeGenOptions.dmaPaddingAlign;
    s.linearColorSpace = m_state.linearColorSpace;
    return s;
}

void DisplayConverter::applySessionSnapshot(const SessionSnapshot &snapshot)
{
    m_state.profileId = snapshot.profileId;
    m_state.displayWidth = snapshot.displayWidth;
    m_state.displayHeight = snapshot.displayHeight;
    m_state.scaleMode = static_cast<DisplayProfile::ScaleMode>(qBound(0, snapshot.scaleMode, 2));
    m_state.dithering = snapshot.dithering;
    m_state.monoThreshold = snapshot.monoThreshold;
    m_state.arrayName = snapshot.arrayName;
    const int storedEncoding = snapshot.encodingMode;
    if (storedEncoding >= 0
        && storedEncoding < static_cast<int>(DisplayCodeGenerator::EncodingMode::Count)) {
        m_state.encodingMode = static_cast<DisplayCodeGenerator::EncodingMode>(storedEncoding);
    } else {
        m_state.encodingMode = PixelFormatCatalog::migrateLegacy(storedEncoding);
    }
    m_state.monoLayout = static_cast<DisplayCodeGenerator::MonoLayout>(qBound(0, snapshot.monoLayout, 2));
    m_state.rotation = snapshot.rotation;
    m_state.flipHorizontal = snapshot.flipHorizontal;
    m_state.flipVertical = snapshot.flipVertical;
    m_state.invertMono = snapshot.invertMono;
    m_state.showGrid = snapshot.showGrid;
    m_state.gridThresholdZoom = snapshot.gridThresholdZoom;
    m_state.filterParams = snapshot.filterParams;
    m_state.filterParams.threshold = m_state.monoThreshold;
    m_state.codeGenOptions.includeHeaderComments = snapshot.codeIncludeComments;
    m_state.codeGenOptions.useProgmem = snapshot.codeUseProgmem;
    m_state.codeGenOptions.staticStorage = snapshot.codeStaticStorage;
    m_state.codeGenOptions.rgb565BigEndian = snapshot.rgb565BigEndian;
    m_state.codeGenOptions.dmaPaddingAlign = qBound(0, snapshot.codeDmaAlign, 8);
    if (m_state.codeGenOptions.dmaPaddingAlign != 4 && m_state.codeGenOptions.dmaPaddingAlign != 8)
        m_state.codeGenOptions.dmaPaddingAlign = 0;
    m_state.linearColorSpace = snapshot.linearColorSpace;
    if (m_state.filterParams.ditherMode == ImageFiltersPipeline::DitherMode::FloydSteinberg)
        m_state.dithering = true;
    else if (m_state.filterParams.ditherMode == ImageFiltersPipeline::DitherMode::None)
        m_state.dithering = false;
    syncProfileIdFromDimensions();
    m_state.colorMode = encodingIsColorMode(static_cast<int>(m_state.encodingMode))
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
    if (!m_state.projectFile.isEmpty())
        m_state.uiState.lastProjectFile = m_state.projectFile.toLocalFile();
    m_session->saveUiState(m_state.uiState);
    emit uiFoldersChanged();
}

QString DisplayConverter::lastOpenImageDir() const
{
    if (!m_state.uiState.lastOpenImageDir.isEmpty())
        return m_state.uiState.lastOpenImageDir;
    return AppPaths::userDocumentsRoot();
}

QString DisplayConverter::lastExportDir() const
{
    if (!m_state.uiState.lastExportDir.isEmpty())
        return m_state.uiState.lastExportDir;
    return AppPaths::exportsDir();
}

void DisplayConverter::applyStoredUiState()
{
    const QString watchOut = m_state.uiState.watchOutputFolder.isEmpty()
        ? AppPaths::watchDir()
        : m_state.uiState.watchOutputFolder;
    if (!m_state.uiState.watchInputFolder.isEmpty() || !watchOut.isEmpty())
        m_watchService.configure(m_state.uiState.watchInputFolder, watchOut);
    if (m_state.uiState.watchActive)
        m_watchService.setActive(true);

    emit uiFoldersChanged();
}

QString DisplayConverter::lastProjectPath() const
{
    if (AppPaths::isInternalDataPath(m_state.uiState.lastProjectFile))
        return {};
    return m_state.uiState.lastProjectFile;
}

bool DisplayConverter::hasRestorableProject() const
{
    const QString path = lastProjectPath();
    return !path.isEmpty() && QFileInfo::exists(path);
}

void DisplayConverter::openUserDocumentsFolder()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(AppPaths::userDocumentsRoot()));
}

QString DisplayConverter::suggestedCodeFilePath() const
{
    QString base = DisplayCodeGenerator::sanitizeIdentifier(m_state.arrayName);
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
    m_state.uiState.lastOpenImageDir = path;
    schedulePersistSession();
}

void DisplayConverter::rememberExportDir(const QString &dir)
{
    const QString path = QFileInfo(dir).isDir() ? dir : QFileInfo(dir).absolutePath();
    if (path.isEmpty())
        return;
    m_state.uiState.lastExportDir = path;
    schedulePersistSession();
}

void DisplayConverter::refreshLocalization()
{
    ++m_state.localizationRevision;
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

void DisplayConverter::rememberOpenSourceInRecent()
{
    if (!m_session || !hasImage() || m_state.sourceFilePath.isEmpty())
        return;
    if (!QFileInfo::exists(m_state.sourceFilePath))
        return;
    m_session->addRecentFile(QUrl::fromLocalFile(m_state.sourceFilePath));
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
    m_state.uiState = SessionUiState{};
    m_state.uiState.watchOutputFolder = AppPaths::watchDir();
    m_watchService.configure(QString(), m_state.uiState.watchOutputFolder);
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
    QString stem = DisplayCodeGenerator::sanitizeIdentifier(m_state.project.name);
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
    if (m_state.projectFile.isEmpty() || !hasImage())
        return;
    saveProject();
}

void DisplayConverter::setOffsetX(int value)
{
    if (m_state.offsetX == value)
        return;
    m_state.offsetX = value;
    emit offsetChanged();
    schedulePersistSession();
}

void DisplayConverter::setOffsetY(int value)
{
    if (m_state.offsetY == value)
        return;
    m_state.offsetY = value;
    emit offsetChanged();
    schedulePersistSession();
}

void DisplayConverter::setShowFullGeneratedCode(bool on)
{
    if (m_state.showFullGeneratedCode == on)
        return;
    m_state.showFullGeneratedCode = on;
    emit showFullGeneratedCodeChanged();
    ImagePipelineController::updateCodePreview(m_state, *this);
}

void DisplayConverter::onWatchExportRequested(const QVariantList &files, const QUrl &targetFile)
{
    enqueueBatchCodeExport(files, targetFile);
}
