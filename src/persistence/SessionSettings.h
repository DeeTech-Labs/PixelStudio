#ifndef PIXELSTUDIO_PERSISTENCE_SESSIONSETTINGS_H
#define PIXELSTUDIO_PERSISTENCE_SESSIONSETTINGS_H

#include <QObject>
#include <QSettings>
#include <QUrl>
#include <QVariantList>

#include "processing/ImageFiltersPipeline.h"
#include "processing/DisplayCodeGenerator.h"

struct SessionSnapshot
{
    QString profileId = QStringLiteral("128x64");
    int displayWidth = 128;
    int displayHeight = 64;
    int scaleMode = 0;
    bool dithering = true;
    int monoThreshold = 128;
    QString arrayName = QStringLiteral("image_data");
    int encodingMode = static_cast<int>(DisplayCodeGenerator::EncodingMode::Mono8HorizontalMsb);
    int monoLayout = static_cast<int>(DisplayCodeGenerator::MonoLayout::RowPacked);
    int rotation = 0;
    bool flipHorizontal = false;
    bool flipVertical = false;
    bool invertMono = false;
    bool showGrid = false;
    int gridThresholdZoom = 8;
    ImageFiltersPipeline::Params filterParams;
    bool codeIncludeComments = true;
    bool codeUseProgmem = true;
    bool codeStaticStorage = true;
};

struct SessionUiState
{
    QString lastProjectFile;
    QString lastOpenImageDir;
    QString lastExportDir;
    QString watchInputFolder;
    QString watchOutputFolder;
    bool watchActive = false;
};

class SessionSettings : public QObject
{
    Q_OBJECT
public:
    explicit SessionSettings(QObject *parent = nullptr);

    void load(SessionSnapshot *snapshot) const;
    void save(const SessionSnapshot &snapshot);

    void loadUiState(SessionUiState *state) const;
    void saveUiState(const SessionUiState &state);

    void addRecentFile(const QUrl &url);
    void pruneMissingRecentFiles();
    QVariantList recentFiles() const;

    void addRecentExport(const QString &absolutePath);
    void pruneMissingRecentExports();
    QVariantList recentExports() const;

    Q_INVOKABLE void resetToDefaults();

signals:
    void recentFilesChanged();
    void recentExportsChanged();

private:
    void syncNow();

    QSettings m_settings;
};

#endif // PIXELSTUDIO_PERSISTENCE_SESSIONSETTINGS_H
