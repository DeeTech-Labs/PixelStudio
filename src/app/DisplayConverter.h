#ifndef DISPLAYCONVERTER_H
#define DISPLAYCONVERTER_H

#include <QObject>
#include <QImage>
#include <QUrl>
#include <QString>
#include <QVariantList>
#include <QFutureWatcher>
#include <QTimer>
#include <QColor>

#include "app/converter/ConverterState.h"
#include "app/converter/ConverterTabSnapshot.h"
#include "processing/DisplayProfile.h"
#include "processing/DisplayRasterizer.h"
#include "processing/DisplayCodeGenerator.h"
#include "processing/ConvertPipeline.h"
#include "processing/ControllerCatalog.h"
#include "processing/EncodingAnalyzer.h"
#include "export/BatchExportService.h"
#include "io/WatchFolderService.h"
#include "persistence/SessionSettings.h"
#include "persistence/ProjectService.h"

class AppSettings;
class ImageLoader;
class PreviewImageProvider;

class DisplayConverter : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QUrl sourcePath READ sourcePath NOTIFY sourcePathChanged)
    Q_PROPERTY(QUrl previewPath READ previewPath NOTIFY previewPathChanged)
    Q_PROPERTY(QUrl processPreviewPath READ processPreviewPath NOTIFY processPreviewPathChanged)
    Q_PROPERTY(bool hasImage READ hasImage NOTIFY hasImageChanged)
    Q_PROPERTY(bool imageLoading READ imageLoading NOTIFY imageLoadingChanged)
    Q_PROPERTY(int sourceWidth READ sourceWidth NOTIFY sourceWidthChanged)
    Q_PROPERTY(int sourceHeight READ sourceHeight NOTIFY sourceHeightChanged)
    Q_PROPERTY(int displayWidth READ displayWidth WRITE setDisplayWidth NOTIFY displayWidthChanged)
    Q_PROPERTY(int displayHeight READ displayHeight WRITE setDisplayHeight NOTIFY displayHeightChanged)
    Q_PROPERTY(QString profileId READ profileId WRITE setProfileId NOTIFY profileIdChanged)
    Q_PROPERTY(int colorMode READ colorMode WRITE setColorMode NOTIFY colorModeChanged)
    Q_PROPERTY(int scaleMode READ scaleMode WRITE setScaleMode NOTIFY scaleModeChanged)
    Q_PROPERTY(bool dithering READ dithering WRITE setDithering NOTIFY ditheringChanged)
    Q_PROPERTY(int monoThreshold READ monoThreshold WRITE setMonoThreshold NOTIFY monoThresholdChanged)
    Q_PROPERTY(QString generatedCode READ generatedCode NOTIFY generatedCodeChanged)
    Q_PROPERTY(QString generatedCodePreview READ generatedCodePreview NOTIFY generatedCodePreviewChanged)
    Q_PROPERTY(bool generatedCodeTruncated READ generatedCodeTruncated NOTIFY generatedCodePreviewChanged)
    Q_PROPERTY(bool showFullGeneratedCode READ showFullGeneratedCode WRITE setShowFullGeneratedCode NOTIFY showFullGeneratedCodeChanged)
    Q_PROPERTY(QString arrayName READ arrayName WRITE setArrayName NOTIFY arrayNameChanged)
    Q_PROPERTY(int dataByteCount READ dataByteCount NOTIFY generatedCodeChanged)
    Q_PROPERTY(QString colorModeName READ colorModeName NOTIFY colorModeChanged)
    Q_PROPERTY(int encodingMode READ encodingMode WRITE setEncodingMode NOTIFY encodingModeChanged)
    Q_PROPERTY(QString encodingModeName READ encodingModeName NOTIFY encodingModeChanged)
    Q_PROPERTY(bool encodingIsMono1Bit READ encodingIsMono1Bit NOTIFY encodingModeChanged)
    Q_PROPERTY(bool encodingIsGrayscale READ encodingIsGrayscale NOTIFY encodingModeChanged)
    Q_PROPERTY(bool encodingIsColor READ encodingIsColor NOTIFY encodingModeChanged)
    Q_PROPERTY(int monoLayout READ monoLayout WRITE setMonoLayout NOTIFY monoLayoutChanged)
    Q_PROPERTY(QString monoLayoutName READ monoLayoutName NOTIFY monoLayoutChanged)
    Q_PROPERTY(int rotation READ rotation WRITE setRotation NOTIFY rotationChanged)
    Q_PROPERTY(bool flipHorizontal READ flipHorizontal WRITE setFlipHorizontal NOTIFY flipHorizontalChanged)
    Q_PROPERTY(bool flipVertical READ flipVertical WRITE setFlipVertical NOTIFY flipVerticalChanged)
    Q_PROPERTY(bool invertMono READ invertMono WRITE setInvertMono NOTIFY invertMonoChanged)
    Q_PROPERTY(bool filterInvert READ filterInvert WRITE setFilterInvert NOTIFY filterInvertChanged)
    Q_PROPERTY(bool hasPreview READ hasPreview NOTIFY previewPathChanged)
    Q_PROPERTY(bool showGrid READ showGrid WRITE setShowGrid NOTIFY showGridChanged)
    Q_PROPERTY(int gridThresholdZoom READ gridThresholdZoom WRITE setGridThresholdZoom NOTIFY gridThresholdZoomChanged)
    Q_PROPERTY(bool batchRunning READ batchRunning NOTIFY batchRunningChanged)
    Q_PROPERTY(int batchProgress READ batchProgress NOTIFY batchProgressChanged)
    Q_PROPERTY(QString projectName READ projectName NOTIFY projectChanged)
    Q_PROPERTY(QUrl projectFile READ projectFile NOTIFY projectChanged)
    Q_PROPERTY(int offsetX READ offsetX WRITE setOffsetX NOTIFY offsetChanged)
    Q_PROPERTY(int offsetY READ offsetY WRITE setOffsetY NOTIFY offsetChanged)
    Q_PROPERTY(QVariantList flashReport READ flashReport NOTIFY flashReportChanged)
    Q_PROPERTY(QVariantList projectAssets READ projectAssets NOTIFY projectChanged)
    Q_PROPERTY(bool watchFolderActive READ watchFolderActive WRITE setWatchFolderActive NOTIFY watchFolderChanged)
    Q_PROPERTY(QString watchInputFolder READ watchInputFolder NOTIFY watchFolderChanged)
    Q_PROPERTY(QString watchOutputFolder READ watchOutputFolder NOTIFY watchFolderChanged)
    Q_PROPERTY(QVariantList recentFiles READ recentFiles NOTIFY recentFilesChanged)
    Q_PROPERTY(QString lastProjectPath READ lastProjectPath NOTIFY uiFoldersChanged)
    Q_PROPERTY(bool hasRestorableProject READ hasRestorableProject NOTIFY uiFoldersChanged)
    Q_PROPERTY(QVariantList recentExports READ recentExports NOTIFY recentExportsChanged)
    Q_PROPERTY(QString lastOpenImageDir READ lastOpenImageDir NOTIFY uiFoldersChanged)
    Q_PROPERTY(QString lastExportDir READ lastExportDir NOTIFY uiFoldersChanged)
    Q_PROPERTY(int localizationRevision READ localizationRevision NOTIFY localizationRevisionChanged)
    Q_PROPERTY(bool blackBackground READ blackBackground WRITE setBlackBackground NOTIFY blackBackgroundChanged)
    Q_PROPERTY(int brightness READ brightness WRITE setBrightness NOTIFY brightnessChanged)
    Q_PROPERTY(int contrast READ contrast WRITE setContrast NOTIFY contrastChanged)
    Q_PROPERTY(int saturation READ saturation WRITE setSaturation NOTIFY saturationChanged)
    Q_PROPERTY(int exposure READ exposure WRITE setExposure NOTIFY exposureChanged)
    Q_PROPERTY(int gamma READ gamma WRITE setGamma NOTIFY gammaChanged)
    Q_PROPERTY(int blur READ blur WRITE setBlur NOTIFY blurChanged)
    Q_PROPERTY(int posterizeRgb READ posterizeRgb WRITE setPosterizeRgb NOTIFY posterizeRgbChanged)
    Q_PROPERTY(bool colorMaskEnabled READ colorMaskEnabled WRITE setColorMaskEnabled NOTIFY colorMaskEnabledChanged)
    Q_PROPERTY(QColor maskColor READ maskColor WRITE setMaskColor NOTIFY maskColorChanged)
    Q_PROPERTY(int maskTolerance READ maskTolerance WRITE setMaskTolerance NOTIFY maskToleranceChanged)
    Q_PROPERTY(int maskAmplify READ maskAmplify WRITE setMaskAmplify NOTIFY maskAmplifyChanged)
    Q_PROPERTY(bool sharpen READ sharpen WRITE setSharpen NOTIFY sharpenChanged)
    Q_PROPERTY(int sobelEdges READ sobelEdges WRITE setSobelEdges NOTIFY sobelEdgesChanged)
    Q_PROPERTY(int posterizeGray READ posterizeGray WRITE setPosterizeGray NOTIFY posterizeGrayChanged)
    Q_PROPERTY(int ditherMode READ ditherMode WRITE setDitherMode NOTIFY ditherModeChanged)
    Q_PROPERTY(int contourMode READ contourMode WRITE setContourMode NOTIFY contourModeChanged)
    Q_PROPERTY(int tonePreset READ tonePreset WRITE setTonePreset NOTIFY tonePresetChanged)
    Q_PROPERTY(bool codeIncludeComments READ codeIncludeComments WRITE setCodeIncludeComments NOTIFY codeGenOptionsChanged)
    Q_PROPERTY(bool codeUseProgmem READ codeUseProgmem WRITE setCodeUseProgmem NOTIFY codeGenOptionsChanged)
    Q_PROPERTY(bool codeStaticStorage READ codeStaticStorage WRITE setCodeStaticStorage NOTIFY codeGenOptionsChanged)
    Q_PROPERTY(bool rgb565BigEndian READ rgb565BigEndian WRITE setRgb565BigEndian NOTIFY rgb565BigEndianChanged)
    Q_PROPERTY(int codeDmaAlign READ codeDmaAlign WRITE setCodeDmaAlign NOTIFY codeDmaAlignChanged)
    Q_PROPERTY(bool linearColorSpace READ linearColorSpace WRITE setLinearColorSpace NOTIFY linearColorSpaceChanged)
    Q_PROPERTY(QVariantList previewPalette READ previewPalette NOTIFY previewPathChanged)
    Q_PROPERTY(int previewColorCount READ previewColorCount NOTIFY previewPathChanged)
    Q_PROPERTY(QString imageFormatName READ imageFormatName NOTIFY hasImageChanged)
    Q_PROPERTY(QString sourceFilePath READ sourceFilePath NOTIFY sourcePathChanged)

public:
    explicit DisplayConverter(SessionSettings *session, AppSettings *appSettings = nullptr, QObject *parent = nullptr);

    void setPreviewProvider(PreviewImageProvider *provider);
    ConverterState &converterState() { return m_state; }
    const ConverterState &converterState() const { return m_state; }

    QUrl sourcePath() const { return m_state.sourcePath; }
    QUrl previewPath() const { return m_state.previewPath; }
    QUrl processPreviewPath() const { return m_state.processPreviewPath; }
    bool hasImage() const { return !m_state.sourceImage.isNull(); }
    bool imageLoading() const { return m_state.imageLoading; }
    bool hasPreview() const { return !m_state.previewPath.isEmpty(); }
    bool batchRunning() const { return m_batchService.running(); }
    int batchProgress() const { return m_batchService.progress(); }
    bool showGrid() const { return m_state.showGrid; }
    int gridThresholdZoom() const { return m_state.gridThresholdZoom; }
    QString projectName() const { return m_state.project.name; }
    QUrl projectFile() const { return m_state.projectFile; }
    QString sourceFilePath() const { return m_state.sourceFilePath; }
    int offsetX() const { return m_state.offsetX; }
    int offsetY() const { return m_state.offsetY; }
    QVariantList flashReport() const { return m_state.flashReport; }
    QVariantList projectAssets() const { return ProjectService::assetsToVariantList(m_state.project.assets); }
    bool watchFolderActive() const { return m_watchService.active(); }
    QString watchInputFolder() const { return m_watchService.inputFolder(); }
    QString watchOutputFolder() const { return m_watchService.outputFolder(); }
    QVariantList recentFiles() const;
    void rememberOpenSourceInRecent();
    QString lastProjectPath() const;
    bool hasRestorableProject() const;
    QString lastOpenImageDir() const;
    QString lastExportDir() const;
    bool showFullGeneratedCode() const { return m_state.showFullGeneratedCode; }
    QString generatedCodePreview() const { return m_state.generatedCodePreview; }
    bool generatedCodeTruncated() const { return m_state.generatedCodeTruncated; }
    int sourceWidth() const;
    int sourceHeight() const;
    int rotation() const { return m_state.rotation; }
    bool flipHorizontal() const { return m_state.flipHorizontal; }
    bool flipVertical() const { return m_state.flipVertical; }
    bool invertMono() const { return m_state.invertMono; }
    bool filterInvert() const { return m_state.filterParams.invert; }
    int displayWidth() const { return m_state.displayWidth; }
    int displayHeight() const { return m_state.displayHeight; }
    QString profileId() const { return m_state.profileId; }
    int colorMode() const { return static_cast<int>(m_state.colorMode); }
    int scaleMode() const { return static_cast<int>(m_state.scaleMode); }
    bool dithering() const { return m_state.dithering; }
    int monoThreshold() const { return m_state.monoThreshold; }
    QString generatedCode() const { return m_state.generatedCode; }
    QString arrayName() const { return m_state.arrayName; }
    int dataByteCount() const;
    QString colorModeName() const;
    int encodingMode() const { return static_cast<int>(m_state.encodingMode); }
    QString encodingModeName() const;
    bool encodingIsMono1Bit() const;
    bool encodingIsGrayscale() const;
    bool encodingIsColor() const;
    int monoLayout() const { return static_cast<int>(m_state.monoLayout); }
    QString monoLayoutName() const;
    bool blackBackground() const { return m_state.filterParams.blackBackground; }
    int brightness() const { return m_state.filterParams.brightness; }
    int contrast() const { return m_state.filterParams.contrast; }
    int saturation() const { return m_state.filterParams.saturation; }
    int exposure() const { return m_state.filterParams.exposure; }
    int gamma() const { return m_state.filterParams.gamma; }
    int blur() const { return m_state.filterParams.blur; }
    int posterizeRgb() const { return m_state.filterParams.posterizeRgb; }
    bool colorMaskEnabled() const { return m_state.filterParams.colorMaskEnabled; }
    QColor maskColor() const { return m_state.filterParams.maskColor; }
    int maskTolerance() const { return m_state.filterParams.maskTolerance; }
    int maskAmplify() const { return m_state.filterParams.maskAmplify; }
    bool sharpen() const { return m_state.filterParams.sharpen; }
    int sobelEdges() const { return m_state.filterParams.sobelEdges; }
    int posterizeGray() const { return m_state.filterParams.posterizeGray; }
    int ditherMode() const { return static_cast<int>(m_state.filterParams.ditherMode); }
    int contourMode() const { return static_cast<int>(m_state.filterParams.contourMode); }
    int tonePreset() const { return static_cast<int>(m_state.filterParams.tonePreset); }
    bool codeIncludeComments() const { return m_state.codeGenOptions.includeHeaderComments; }
    bool codeUseProgmem() const { return m_state.codeGenOptions.useProgmem; }
    bool codeStaticStorage() const { return m_state.codeGenOptions.staticStorage; }
    bool rgb565BigEndian() const { return m_state.codeGenOptions.rgb565BigEndian; }
    int codeDmaAlign() const { return m_state.codeGenOptions.dmaPaddingAlign; }
    QVariantList previewPalette() const;
    int previewColorCount() const;
    QString imageFormatName() const;
    bool linearColorSpace() const { return m_state.linearColorSpace; }

    Q_INVOKABLE void setArrayName(const QString &name);
    Q_INVOKABLE void setDisplayWidth(int w);
    Q_INVOKABLE void setDisplayHeight(int h);
    Q_INVOKABLE void setProfileId(const QString &id);
    Q_INVOKABLE void setColorMode(int mode);
    Q_INVOKABLE void setScaleMode(int mode);
    Q_INVOKABLE void setDithering(bool on);
    Q_INVOKABLE void setMonoThreshold(int value);
    Q_INVOKABLE void setMonoLayout(int mode);
    Q_INVOKABLE void setEncodingMode(int mode);
    Q_INVOKABLE void setBlackBackground(bool on);
    Q_INVOKABLE void setBrightness(int value);
    Q_INVOKABLE void setContrast(int value);
    Q_INVOKABLE void setSaturation(int value);
    Q_INVOKABLE void setExposure(int value);
    Q_INVOKABLE void setGamma(int value);
    Q_INVOKABLE void setBlur(int value);
    Q_INVOKABLE void setPosterizeRgb(int value);
    Q_INVOKABLE void setColorMaskEnabled(bool on);
    Q_INVOKABLE void setMaskColor(const QColor &color);
    Q_INVOKABLE void setMaskTolerance(int value);
    Q_INVOKABLE void setMaskAmplify(int value);
    Q_INVOKABLE void setSharpen(bool on);
    Q_INVOKABLE void setSobelEdges(int value);
    Q_INVOKABLE void setPosterizeGray(int value);
    Q_INVOKABLE void setDitherMode(int mode);
    Q_INVOKABLE void setContourMode(int mode);
    Q_INVOKABLE void setTonePreset(int preset);
    Q_INVOKABLE void resetFilters();
    Q_INVOKABLE void resetTransform();
    Q_INVOKABLE void centerOffsetOnDisplay();
    Q_INVOKABLE void copyGeneratedArray();
    Q_INVOKABLE void setCodeIncludeComments(bool on);
    Q_INVOKABLE void setCodeUseProgmem(bool on);
    Q_INVOKABLE void setCodeStaticStorage(bool on);
    Q_INVOKABLE void setRgb565BigEndian(bool on);
    Q_INVOKABLE void setCodeDmaAlign(int align);
    Q_INVOKABLE void setLinearColorSpace(bool on);
    Q_INVOKABLE void setShowGrid(bool on);
    Q_INVOKABLE void setGridThresholdZoom(int value);
    Q_INVOKABLE void setRotation(int degrees);
    Q_INVOKABLE void rotateClockwise();
    Q_INVOKABLE void setFlipHorizontal(bool on);
    Q_INVOKABLE void setFlipVertical(bool on);
    Q_INVOKABLE void setInvertMono(bool on);
    Q_INVOKABLE void setFilterInvert(bool on);
    Q_INVOKABLE void setOffsetX(int value);
    Q_INVOKABLE void setOffsetY(int value);
    Q_INVOKABLE void setShowFullGeneratedCode(bool on);
    Q_INVOKABLE void swapDisplayDimensions();
    Q_INVOKABLE bool loadImage(const QUrl &url);
    Q_INVOKABLE bool loadRecentFile(const QString &localPath);
    Q_INVOKABLE bool restoreLastProject();
    Q_INVOKABLE bool loadFromClipboard();
    Q_INVOKABLE void loadFromUrl(const QString &urlString);
    Q_INVOKABLE void clear();
    Q_INVOKABLE void refresh();
    Q_INVOKABLE QVariantList displayPresets() const;
    Q_INVOKABLE QVariantList availableEncodingModes() const;
    Q_INVOKABLE QVariantList availableEncodingModesForUi() const;
    Q_INVOKABLE QVariantList availableBasicEncodingModes() const;
    Q_INVOKABLE void applyBasicEncoding(int mode, int monoLayout);
    Q_INVOKABLE QVariantList workflowPresets() const;
    Q_INVOKABLE void applyWorkflowPreset(const QString &id);
    Q_INVOKABLE void newProject(const QString &name);
    Q_INVOKABLE bool openProject(const QUrl &url);
    Q_INVOKABLE bool saveProject();
    Q_INVOKABLE bool saveProjectAs(const QUrl &url);
    Q_INVOKABLE bool importHeader(const QUrl &url);
    Q_INVOKABLE void configureWatchFolder(const QString &inputFolder, const QString &outputFolder);
    Q_INVOKABLE void configureWatchFolders(const QUrl &inputFolder, const QUrl &outputFolder);
    Q_INVOKABLE void setWatchFolderActive(bool active);
    Q_INVOKABLE bool buildSpriteAtlas(const QVariantList &urls, const QUrl &targetFile, int frameWidth, int frameHeight);
    Q_INVOKABLE void copyToClipboard(const QString &text);
    Q_INVOKABLE bool saveCodeToFile(const QUrl &url);
    Q_INVOKABLE bool saveBinaryToFile(const QUrl &url);
    Q_INVOKABLE void enqueueBatchCodeExport(const QVariantList &urls, const QUrl &targetFile);
    Q_INVOKABLE void cancelBatchExport();
    Q_INVOKABLE void openUserDocumentsFolder();
    Q_INVOKABLE void openAppDataFolder();
    Q_INVOKABLE void resetSession();
    Q_INVOKABLE bool exportSettingsTo(const QUrl &folderUrl);
    Q_INVOKABLE bool importSettingsFrom(const QUrl &folderUrl);
    Q_INVOKABLE void flushPersistence();
    Q_INVOKABLE QVariantList recentExports() const;
    Q_INVOKABLE QString suggestedCodeFilePath() const;
    Q_INVOKABLE QUrl suggestedCodeFileUrl() const;
    Q_INVOKABLE void rememberOpenImageDir(const QString &dir);
    Q_INVOKABLE void rememberExportDir(const QString &dir);
    Q_INVOKABLE void refreshLocalization();
    void applyProject(const StudioProject &project);
    StudioProject projectSnapshot() const;
    StudioProject projectSnapshotForDisk() const;
    void applySessionSnapshot(const SessionSnapshot &snapshot);
    void markOrientedDirty();
    void refreshSourcePreview();
    void updateWatchExportPrefix();
    void restartAutosaveTimer();
    ConverterTabSnapshot captureTabState() const;
    void restoreTabState(const ConverterTabSnapshot &snapshot);

    QTimer *rebuildDebounceTimer() { return &m_rebuildDebounceTimer; }
    QFutureWatcher<ConverterAsyncBuildResult> *rebuildWatcher() { return &m_rebuildWatcher; }
    ImageLoader *loader() const { return m_loader; }
    SessionSettings *session() const { return m_session; }
    WatchFolderService *watchService() { return &m_watchService; }
    BatchExportService *batchService() { return &m_batchService; }
    QImage orientedSource() const;
    void schedulePersistSession();
    void applyPipelineResult(const ConverterAsyncBuildResult &result);
    ConvertPipelineParams pipelineParams() const;
    SessionSnapshot sessionSnapshot() const;

    int localizationRevision() const { return m_state.localizationRevision; }

signals:
    void localizationRevisionChanged();
    void sourcePathChanged();
    void previewPathChanged();
    void processPreviewPathChanged();
    void hasImageChanged();
    void imageLoadingChanged();
    void sourceWidthChanged();
    void sourceHeightChanged();
    void displayWidthChanged();
    void displayHeightChanged();
    void profileIdChanged();
    void colorModeChanged();
    void scaleModeChanged();
    void ditheringChanged();
    void monoThresholdChanged();
    void blackBackgroundChanged();
    void brightnessChanged();
    void contrastChanged();
    void saturationChanged();
    void exposureChanged();
    void gammaChanged();
    void blurChanged();
    void posterizeRgbChanged();
    void colorMaskEnabledChanged();
    void maskColorChanged();
    void maskToleranceChanged();
    void maskAmplifyChanged();
    void sharpenChanged();
    void sobelEdgesChanged();
    void posterizeGrayChanged();
    void ditherModeChanged();
    void contourModeChanged();
    void tonePresetChanged();
    void codeGenOptionsChanged();
    void rgb565BigEndianChanged();
    void codeDmaAlignChanged();
    void linearColorSpaceChanged();
    void showGridChanged();
    void gridThresholdZoomChanged();
    void generatedCodeChanged();
    void generatedCodePreviewChanged();
    void showFullGeneratedCodeChanged();
    void arrayNameChanged();
    void encodingModeChanged();
    void monoLayoutChanged();
    void rotationChanged();
    void flipHorizontalChanged();
    void flipVerticalChanged();
    void invertMonoChanged();
    void filterInvertChanged();
    void projectChanged();
    void offsetChanged();
    void flashReportChanged();
    void watchFolderChanged();
    void recentFilesChanged();
    void recentExportsChanged();
    void uiFoldersChanged();
    void errorOccurred(const QString &message);
    void batchRunningChanged();
    void batchProgressChanged();

private Q_SLOTS:
    void onImageLoaded(const QImage &image, const QUrl &sourceUrl);
    void onBatchFinished(bool ok, const QString &errorMessage);
    void onWatchExportRequested(const QVariantList &files, const QUrl &targetFile);
    void persistSession();

private:
    void applyProfile(const DisplayProfile &profile);
    void syncProfileIdFromDimensions();
    void markToneCustom();
    void emitAllFilterSignals();
    DisplayCodeGenerator::CodeGenOptions codeGenOptions() const;
    void rebuild();
    void scheduleRebuild(bool immediate = false);
    QUrl publishPreview(const QString &slotName, const QImage &img);
    void applyStoredUiState();
    void persistUiState();
    void onAutosaveTimeout();

    ImageLoader *m_loader;
    PreviewImageProvider *m_previewProvider = nullptr;
    SessionSettings *m_session = nullptr;
    AppSettings *m_appSettings = nullptr;
    BatchExportService m_batchService;
    WatchFolderService m_watchService;
    ConverterState m_state;
    QTimer m_rebuildDebounceTimer;
    QTimer m_sessionSaveTimer;
    QTimer m_autosaveTimer;
    QFutureWatcher<ConverterAsyncBuildResult> m_rebuildWatcher;
};

#endif // DISPLAYCONVERTER_H
