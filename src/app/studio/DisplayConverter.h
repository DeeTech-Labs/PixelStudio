#ifndef DISPLAYCONVERTER_H
#define DISPLAYCONVERTER_H

#include <QObject>
#include <QImage>
#include <QUrl>
#include <QString>
#include <QVariantList>
#include <QFutureWatcher>
#include <QTimer>

#include "app/studio/model/ConverterState.h"
#include "app/studio/model/ConverterTabSnapshot.h"
#include "app/studio/controllers/image/ImageFilterController.h"
#include "app/studio/controllers/image/ImageTransformController.h"
#include "app/studio/controllers/output/DisplayOutputController.h"
#include "app/studio/controllers/output/CodeGenController.h"
#include "app/studio/controllers/output/ViewportController.h"
#include "app/studio/controllers/project/ProjectController.h"
#include "app/studio/controllers/export/ExportController.h"
#include "processing/DisplayRasterizer.h"
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
    Q_PROPERTY(QString generatedCode READ generatedCode NOTIFY generatedCodeChanged)
    Q_PROPERTY(QString generatedCodePreview READ generatedCodePreview NOTIFY generatedCodePreviewChanged)
    Q_PROPERTY(bool generatedCodeTruncated READ generatedCodeTruncated NOTIFY generatedCodePreviewChanged)
    Q_PROPERTY(bool hasPreview READ hasPreview NOTIFY previewPathChanged)
    Q_PROPERTY(QVariantList flashReport READ flashReport NOTIFY flashReportChanged)
    Q_PROPERTY(int localizationRevision READ localizationRevision NOTIFY localizationRevisionChanged)
    Q_PROPERTY(QVariantList previewPalette READ previewPalette NOTIFY previewPathChanged)
    Q_PROPERTY(int previewColorCount READ previewColorCount NOTIFY previewPathChanged)
    Q_PROPERTY(QString imageFormatName READ imageFormatName NOTIFY hasImageChanged)
    Q_PROPERTY(QString sourceFilePath READ sourceFilePath NOTIFY sourcePathChanged)

public:
    explicit DisplayConverter(SessionSettings *session, AppSettings *appSettings = nullptr, QObject *parent = nullptr);

    void setPreviewProvider(PreviewImageProvider *provider);
    PreviewImageProvider *previewProvider() const { return m_previewProvider; }
    void setActiveTabId(const QString &tabId);
    QString activeTabId() const { return m_activeTabId; }
    AppSettings *appSettings() const { return m_appSettings; }

    ImageFilterController *imageFilters() { return &m_filters; }
    ImageTransformController *imageTransform() { return &m_transform; }
    DisplayOutputController *displayOutput() { return &m_displayOutput; }
    CodeGenController *codeGen() { return &m_codeGen; }
    ViewportController *viewport() { return &m_viewport; }
    ProjectController *project() { return &m_project; }
    const ProjectController *project() const { return &m_project; }
    ExportController *exportPanel() { return &m_export; }

    ConverterState &converterState() { return m_state; }
    const ConverterState &converterState() const { return m_state; }

    QUrl sourcePath() const { return m_state.sourcePath; }
    QUrl previewPath() const { return m_state.previewPath; }
    QUrl processPreviewPath() const { return m_state.processPreviewPath; }
    bool hasImage() const { return !m_state.sourceImage.isNull(); }
    bool imageLoading() const { return m_state.imageLoading; }
    bool hasPreview() const { return !m_state.previewPath.isEmpty(); }
    QString sourceFilePath() const { return m_state.sourceFilePath; }
    QVariantList flashReport() const { return m_state.flashReport; }
    void rememberOpenSourceInRecent();
    QString generatedCodePreview() const { return m_state.generatedCodePreview; }
    bool generatedCodeTruncated() const { return m_state.generatedCodeTruncated; }
    int sourceWidth() const;
    int sourceHeight() const;
    QString generatedCode() const { return m_state.generatedCode; }
    QVariantList previewPalette() const;
    int previewColorCount() const;
    QString imageFormatName() const;
    int localizationRevision() const { return m_state.localizationRevision; }

    Q_INVOKABLE bool loadImage(const QUrl &url);
    Q_INVOKABLE bool loadFromClipboard();
    Q_INVOKABLE void loadFromUrl(const QString &urlString);
    Q_INVOKABLE void clear();
    void detachActiveTab();
    Q_INVOKABLE void refreshLocalization();
    void flushPersistence();
    void reloadImportedSettings();
    void applyDefaultGridPreference();

    void applyProject(const StudioProject &project);
    StudioProject projectSnapshot() const;
    StudioProject projectSnapshotForDisk() const;
    void applySessionSnapshot(const SessionSnapshot &snapshot);
    void markOrientedDirty();
    void refreshSourcePreview();
    void notifySourceGeometryChanged();
    void notifyAllToolsChanged();
    void updateWatchExportPrefix();
    void restartAutosaveTimer();
    ConverterTabSnapshot captureTabState(const QString &tabId = QString()) const;
    void restoreTabState(const ConverterTabSnapshot &snapshot);

    QTimer *rebuildDebounceTimer() { return &m_rebuildDebounceTimer; }
    QFutureWatcher<ConverterAsyncBuildResult> *rebuildWatcher() { return &m_rebuildWatcher; }
    ImageLoader *loader() const { return m_loader; }
    SessionSettings *session() const { return m_session; }
    QImage orientedSource() const;
    void schedulePersistSession();
    void applyPipelineResult(const ConverterAsyncBuildResult &result);
    ConvertPipelineParams pipelineParams() const;
    SessionSnapshot sessionSnapshot() const;

signals:
    void localizationRevisionChanged();
    void sourcePathChanged();
    void previewPathChanged();
    void processPreviewPathChanged();
    void hasImageChanged();
    void imageLoadingChanged();
    void sourceWidthChanged();
    void sourceHeightChanged();
    void generatedCodeChanged();
    void generatedCodePreviewChanged();
    void flashReportChanged();
    void errorOccurred(const QString &message);

private Q_SLOTS:
    void onImageLoaded(const QImage &image, const QUrl &sourceUrl);
    void persistSession();

private:
    void rebuild();
    void scheduleRebuild(bool immediate = false);
    void persistUiState();
    void loadPersistedSession();
    void onAutosaveTimeout();

    ImageLoader *m_loader;
    PreviewImageProvider *m_previewProvider = nullptr;
    QString m_activeTabId;
    SessionSettings *m_session = nullptr;
    AppSettings *m_appSettings = nullptr;
    ConverterState m_state;
    ImageFilterController m_filters;
    ImageTransformController m_transform;
    DisplayOutputController m_displayOutput;
    CodeGenController m_codeGen;
    ViewportController m_viewport;
    ProjectController m_project;
    ExportController m_export;
    QTimer m_rebuildDebounceTimer;
    QTimer m_sessionSaveTimer;
    QTimer m_autosaveTimer;
    QFutureWatcher<ConverterAsyncBuildResult> m_rebuildWatcher;
};

#endif // DISPLAYCONVERTER_H
