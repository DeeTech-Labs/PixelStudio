#ifndef DISPLAYCONVERTER_H
#define DISPLAYCONVERTER_H

#include <QObject>
#include <QImage>
#include <QUrl>
#include <QString>
#include <QVariantList>
#include <QTemporaryFile>
#include <QFutureWatcher>
#include <QTimer>
#include <QColor>

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

class DisplayConverter : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QUrl sourcePath READ sourcePath NOTIFY sourcePathChanged)
    Q_PROPERTY(QUrl previewPath READ previewPath NOTIFY previewPathChanged)
    Q_PROPERTY(QUrl processPreviewPath READ processPreviewPath NOTIFY processPreviewPathChanged)
    Q_PROPERTY(bool hasImage READ hasImage NOTIFY hasImageChanged)
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
    Q_PROPERTY(    int encodingMode READ encodingMode WRITE setEncodingMode NOTIFY encodingModeChanged)
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

public:
    explicit DisplayConverter(SessionSettings *session, AppSettings *appSettings = nullptr, QObject *parent = nullptr);

    QUrl sourcePath() const { return m_sourcePath; }
    QUrl previewPath() const { return m_previewPath; }
    QUrl processPreviewPath() const { return m_processPreviewPath; }
    bool hasImage() const { return !m_sourceImage.isNull(); }
    bool hasPreview() const { return !m_previewPath.isEmpty(); }
    bool batchRunning() const { return m_batchService.running(); }
    int batchProgress() const { return m_batchService.progress(); }
    bool showGrid() const { return m_showGrid; }
    int gridThresholdZoom() const { return m_gridThresholdZoom; }
    QString projectName() const { return m_project.name; }
    QUrl projectFile() const { return m_projectFile; }
    int offsetX() const { return m_offsetX; }
    int offsetY() const { return m_offsetY; }
    QVariantList flashReport() const { return m_flashReport; }
    QVariantList projectAssets() const { return ProjectService::assetsToVariantList(m_project.assets); }
    bool watchFolderActive() const { return m_watchService.active(); }
    QString watchInputFolder() const { return m_watchService.inputFolder(); }
    QString watchOutputFolder() const { return m_watchService.outputFolder(); }
    QVariantList recentFiles() const;
    QString lastProjectPath() const;
    bool hasRestorableProject() const;
    QString lastOpenImageDir() const;
    QString lastExportDir() const;
    bool showFullGeneratedCode() const { return m_showFullGeneratedCode; }
    QString generatedCodePreview() const { return m_generatedCodePreview; }
    bool generatedCodeTruncated() const { return m_generatedCodeTruncated; }
    int sourceWidth() const;
    int sourceHeight() const;
    int rotation() const { return m_rotation; }
    bool flipHorizontal() const { return m_flipHorizontal; }
    bool flipVertical() const { return m_flipVertical; }
    bool invertMono() const { return m_invertMono; }
    bool filterInvert() const { return m_filterParams.invert; }
    int displayWidth() const { return m_displayWidth; }
    int displayHeight() const { return m_displayHeight; }
    QString profileId() const { return m_profileId; }
    int colorMode() const { return static_cast<int>(m_colorMode); }
    int scaleMode() const { return static_cast<int>(m_scaleMode); }
    bool dithering() const { return m_dithering; }
    int monoThreshold() const { return m_monoThreshold; }
    QString generatedCode() const { return m_generatedCode; }
    QString arrayName() const { return m_arrayName; }
    int dataByteCount() const;
    QString colorModeName() const;
    int encodingMode() const { return static_cast<int>(m_encodingMode); }
    QString encodingModeName() const;
    bool encodingIsMono1Bit() const;
    bool encodingIsGrayscale() const;
    bool encodingIsColor() const;
    int monoLayout() const { return static_cast<int>(m_monoLayout); }
    QString monoLayoutName() const;
    bool blackBackground() const { return m_filterParams.blackBackground; }
    int brightness() const { return m_filterParams.brightness; }
    int contrast() const { return m_filterParams.contrast; }
    int saturation() const { return m_filterParams.saturation; }
    int exposure() const { return m_filterParams.exposure; }
    int gamma() const { return m_filterParams.gamma; }
    int blur() const { return m_filterParams.blur; }
    int posterizeRgb() const { return m_filterParams.posterizeRgb; }
    bool colorMaskEnabled() const { return m_filterParams.colorMaskEnabled; }
    QColor maskColor() const { return m_filterParams.maskColor; }
    int maskTolerance() const { return m_filterParams.maskTolerance; }
    int maskAmplify() const { return m_filterParams.maskAmplify; }
    bool sharpen() const { return m_filterParams.sharpen; }
    int sobelEdges() const { return m_filterParams.sobelEdges; }
    int posterizeGray() const { return m_filterParams.posterizeGray; }
    int ditherMode() const { return static_cast<int>(m_filterParams.ditherMode); }
    int contourMode() const { return static_cast<int>(m_filterParams.contourMode); }
    int tonePreset() const { return static_cast<int>(m_filterParams.tonePreset); }
    bool codeIncludeComments() const { return m_codeGenOptions.includeHeaderComments; }
    bool codeUseProgmem() const { return m_codeGenOptions.useProgmem; }
    bool codeStaticStorage() const { return m_codeGenOptions.staticStorage; }

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

    int localizationRevision() const { return m_localizationRevision; }

signals:
    void localizationRevisionChanged();
    void sourcePathChanged();
    void previewPathChanged();
    void processPreviewPathChanged();
    void hasImageChanged();
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
    struct AsyncBuildResult {
        quint64 generation = 0;
        DisplayRasterizer::Result result;
        QString generatedCode;
    };

    void applyProfile(const DisplayProfile &profile);
    void syncProfileIdFromDimensions();
    void markToneCustom();
    void emitAllFilterSignals();
    DisplayCodeGenerator::CodeGenOptions codeGenOptions() const;
    void rebuild();
    void scheduleRebuild(bool immediate = false);
    void startAsyncRebuild();
    void onAsyncRebuildFinished();
    QImage orientedSource() const;
    void markOrientedDirty();
    void refreshSourcePreview();
    QUrl writeTempPreview(const QImage &img, QTemporaryFile **slot);
    ConvertPipelineParams pipelineParams() const;
    SessionSnapshot sessionSnapshot() const;
    void applySessionSnapshot(const SessionSnapshot &snapshot);
    void schedulePersistSession();
    void updateCodePreview();
    void updateFlashReport();
    void applyProject(const StudioProject &project);
    void applyStoredUiState();
    void persistUiState();
    void updateWatchExportPrefix();
    void restartAutosaveTimer();
    void onAutosaveTimeout();

    ImageLoader *m_loader;
    SessionSettings *m_session = nullptr;
    AppSettings *m_appSettings = nullptr;
    BatchExportService m_batchService;
    WatchFolderService m_watchService;
    QImage m_sourceImage;
    QUrl m_sourcePath;
    QUrl m_previewPath;
    QUrl m_processPreviewPath;
    QString m_generatedCode;
    QString m_generatedCodePreview;
    bool m_generatedCodeTruncated = false;
    bool m_showFullGeneratedCode = false;
    DisplayRasterizer::Result m_lastResult;
    QTemporaryFile *m_sourceTemp = nullptr;
    QTemporaryFile *m_previewTemp = nullptr;
    QTemporaryFile *m_processTemp = nullptr;

    QString m_profileId = QStringLiteral("128x64");
    int m_displayWidth = 128;
    int m_displayHeight = 64;
    DisplayProfile::ColorMode m_colorMode = DisplayProfile::Mono1Bit;
    DisplayProfile::ScaleMode m_scaleMode = DisplayProfile::Fit;
    bool m_dithering = true;
    int m_monoThreshold = 128;
    QString m_arrayName = QStringLiteral("image_data");
    DisplayCodeGenerator::MonoLayout m_monoLayout = DisplayCodeGenerator::MonoLayout::RowPacked;
    DisplayCodeGenerator::EncodingMode m_encodingMode = DisplayCodeGenerator::EncodingMode::Mono8HorizontalMsb;
    int m_rotation = 0;
    bool m_flipHorizontal = false;
    bool m_flipVertical = false;
    bool m_invertMono = false;
    ImageFiltersPipeline::Params m_filterParams;
    DisplayCodeGenerator::CodeGenOptions m_codeGenOptions;
    mutable QImage m_orientedCache;
    mutable bool m_orientedDirty = true;

    QTimer m_rebuildDebounceTimer;
    QTimer m_sessionSaveTimer;
    QTimer m_autosaveTimer;
    QFutureWatcher<AsyncBuildResult> m_rebuildWatcher;
    quint64 m_nextGeneration = 0;
    quint64 m_lastAppliedGeneration = 0;
    bool m_rebuildPending = false;
    bool m_showGrid = false;
    int m_gridThresholdZoom = 8;
    StudioProject m_project;
    QUrl m_projectFile;
    int m_offsetX = 0;
    int m_offsetY = 0;
    QVariantList m_flashReport;
    SessionUiState m_uiState;
    int m_localizationRevision = 0;
};

#endif // DISPLAYCONVERTER_H
