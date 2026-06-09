#include "app/studio/controllers/export/ExportController.h"
#include "app/studio/DisplayConverter.h"

#include "export/BatchExportService.h"
#include "export/SpriteAtlasService.h"
#include "io/WatchFolderService.h"
#include "translation/AppLocale.h"
#include "io/BinaryExporter.h"
#include "persistence/AppPaths.h"
#include "persistence/SessionSettings.h"
#include "processing/DisplayCodeGenerator.h"
#include "LogCategories.h"

#include <QClipboard>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QTextStream>
#include <QUrl>

ExportController::ExportController(QObject *parent)
    : QObject(parent)
{
}

void ExportController::attach(DisplayConverter *host, ConverterState *state, SessionSettings *session)
{
    m_host = host;
    m_state = state;
    m_session = session;

    connect(&m_batchService, &BatchExportService::runningChanged, this, &ExportController::batchRunningChanged);
    connect(&m_batchService, &BatchExportService::progressChanged, this, &ExportController::batchProgressChanged);
    connect(&m_batchService, &BatchExportService::finished, this, &ExportController::onBatchFinished);
    connect(&m_watchService, &WatchFolderService::activeChanged, this, &ExportController::watchFolderChanged);
    connect(&m_watchService, &WatchFolderService::foldersChanged, this, &ExportController::watchFolderChanged);
    connect(&m_watchService, &WatchFolderService::filesChanged, this, &ExportController::watchFolderChanged);
    connect(&m_watchService, &WatchFolderService::exportRequested, this, &ExportController::onWatchExportRequested);
    if (m_session) {
        connect(m_session, &SessionSettings::recentExportsChanged, this, &ExportController::recentExportsChanged);
    }
}

void ExportController::resetBatchExport()
{
    m_batchService.reset();
}

bool ExportController::batchRunning() const
{
    return m_batchService.running();
}

int ExportController::batchProgress() const
{
    return m_batchService.progress();
}

bool ExportController::watchFolderActive() const
{
    return m_watchService.active();
}

QString ExportController::watchInputFolder() const
{
    return m_watchService.inputFolder();
}

QString ExportController::watchOutputFolder() const
{
    return m_watchService.outputFolder();
}

QVariantList ExportController::recentExports() const
{
    return m_session ? m_session->recentExports() : QVariantList{};
}

QString ExportController::lastExportDir() const
{
    if (m_state && !m_state->uiState.lastExportDir.isEmpty())
        return m_state->uiState.lastExportDir;
    return AppPaths::exportsDir();
}

void ExportController::applyStoredWatchState()
{
    if (!m_state)
        return;
    const QString watchOut = m_state->uiState.watchOutputFolder.isEmpty()
        ? AppPaths::watchDir()
        : m_state->uiState.watchOutputFolder;
    if (!m_state->uiState.watchInputFolder.isEmpty() || !watchOut.isEmpty())
        m_watchService.configure(m_state->uiState.watchInputFolder, watchOut);
    if (m_state->uiState.watchActive)
        m_watchService.setActive(true);
    emit uiFoldersChanged();
}

void ExportController::configureWatchFolder(const QString &inputFolder, const QString &outputFolder)
{
    if (!m_host || !m_state)
        return;

    QString out = outputFolder.trimmed();
    if (out.isEmpty())
        out = AppPaths::watchDir();
    m_state->uiState.watchInputFolder = inputFolder;
    m_state->uiState.watchOutputFolder = out;
    m_watchService.configure(inputFolder, out);
    m_host->updateWatchExportPrefix();
    m_host->schedulePersistSession();
    emit watchFolderChanged();
}

void ExportController::configureWatchFolders(const QUrl &inputFolder, const QUrl &outputFolder)
{
    configureWatchFolder(inputFolder.toLocalFile(), outputFolder.toLocalFile());
}

void ExportController::setWatchFolderActive(bool active)
{
    if (!m_host || !m_state)
        return;
    m_state->uiState.watchActive = active;
    m_watchService.setActive(active);
    m_host->schedulePersistSession();
    emit watchFolderChanged();
}

bool ExportController::buildSpriteAtlas(const QVariantList &urls,
                                        const QUrl &targetFile,
                                        int frameWidth,
                                        int frameHeight)
{
    if (!m_host || !m_state)
        return false;

    SpriteAtlasRequest request;
    request.pipeline = m_host->pipelineParams();
    request.arrayPrefix = m_state->arrayName.isEmpty() ? QStringLiteral("sprite") : m_state->arrayName;
    request.frameWidth = frameWidth;
    request.frameHeight = frameHeight;
    request.fixedGrid = true;
    request.padding = 1;
    request.encodingMode = m_state->encodingMode;
    request.monoLayout = m_state->monoLayout;
    request.codeGenOptions = m_state->codeGenOptions;
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

void ExportController::copyToClipboard(const QString &text)
{
    QGuiApplication::clipboard()->setText(text);
}

bool ExportController::saveCodeToFile(const QUrl &url)
{
    if (!m_host || !m_state)
        return false;
    if (m_state->generatedCode.isEmpty())
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
    out << m_state->generatedCode;
    rememberExportDir(path);
    if (m_session)
        m_session->addRecentExport(path);
    return true;
}

bool ExportController::saveBinaryToFile(const QUrl &url)
{
    if (!m_host || !m_state)
        return false;
    if (m_state->lastResult.width < 1 || m_state->lastResult.height < 1)
        return false;
    QString path = url.toLocalFile();
    if (path.isEmpty())
        path = url.path();
    if (path.isEmpty()) {
        emit errorOccurred(AppLocale::tr("Specify a file path"));
        return false;
    }

    const QByteArray data = DisplayCodeGenerator::binaryData(
        m_state->encodingMode,
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
        m_state->codeGenOptions);

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

void ExportController::enqueueBatchCodeExport(const QVariantList &urls, const QUrl &targetFile)
{
    if (!m_host || !m_state)
        return;

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
    job.pipeline = m_host->pipelineParams();
    job.profileId = m_state->profileId;
    job.encodingMode = m_state->encodingMode;
    job.monoLayout = m_state->monoLayout;
    job.codeGenOptions = m_state->codeGenOptions;
    m_batchService.enqueue(job);
}

void ExportController::cancelBatchExport()
{
    m_batchService.cancel();
}

void ExportController::setExportNamePrefix(const QString &stem)
{
    m_watchService.setExportNamePrefix(stem);
}

QString ExportController::suggestedCodeFilePath() const
{
    if (!m_state)
        return {};
    QString base = DisplayCodeGenerator::sanitizeIdentifier(m_state->arrayName);
    if (base.isEmpty())
        base = QStringLiteral("image_data");
    const QString dir = lastExportDir();
    return dir + QLatin1Char('/') + base + QStringLiteral(".h");
}

QUrl ExportController::suggestedCodeFileUrl() const
{
    return QUrl::fromLocalFile(suggestedCodeFilePath());
}

void ExportController::rememberExportDir(const QString &dir)
{
    if (!m_host || !m_state)
        return;
    const QString path = QFileInfo(dir).isDir() ? dir : QFileInfo(dir).absolutePath();
    if (path.isEmpty())
        return;
    m_state->uiState.lastExportDir = path;
    m_host->schedulePersistSession();
}

void ExportController::onBatchFinished(bool ok, const QString &errorMessage)
{
    if (ok) {
        qCInfo(lcExport) << "Batch export finished";
        return;
    }
    if (!errorMessage.isEmpty()) {
        qCWarning(lcExport) << "Batch export failed:" << errorMessage;
        emit errorOccurred(errorMessage);
    }
}

void ExportController::onWatchExportRequested(const QVariantList &files, const QUrl &targetFile)
{
    enqueueBatchCodeExport(files, targetFile);
}

void ExportController::notifyWatchFolderChanged()
{
    emit watchFolderChanged();
}

void ExportController::notifyUiFoldersChanged()
{
    emit uiFoldersChanged();
}

void ExportController::notifyRecentExportsChanged()
{
    emit recentExportsChanged();
}
