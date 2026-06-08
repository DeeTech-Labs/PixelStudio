#include "app/converter/ExportController.h"
#include "app/DisplayConverter.h"

#include "export/BatchExportService.h"
#include "export/SpriteAtlasService.h"
#include "i18n/AppLocale.h"
#include "io/BinaryExporter.h"
#include "persistence/AppPaths.h"
#include "processing/DisplayCodeGenerator.h"

#include <QFile>
#include <QTextStream>
#include <QUrl>

void ExportController::configureWatchFolder(DisplayConverter &converter,
                                            const QString &inputFolder,
                                            const QString &outputFolder)
{
    QString out = outputFolder.trimmed();
    if (out.isEmpty())
        out = AppPaths::watchDir();
    converter.m_uiState.watchInputFolder = inputFolder;
    converter.m_uiState.watchOutputFolder = out;
    converter.m_watchService.configure(inputFolder, out);
    converter.updateWatchExportPrefix();
    converter.schedulePersistSession();
}

void ExportController::setWatchFolderActive(DisplayConverter &converter, bool active)
{
    converter.m_uiState.watchActive = active;
    converter.m_watchService.setActive(active);
    converter.schedulePersistSession();
}

bool ExportController::buildSpriteAtlas(DisplayConverter &converter,
                                        const QVariantList &urls,
                                        const QUrl &targetFile,
                                        int frameWidth,
                                        int frameHeight)
{
    SpriteAtlasRequest request;
    request.pipeline = converter.pipelineParams();
    request.arrayPrefix = converter.m_arrayName.isEmpty() ? QStringLiteral("sprite") : converter.m_arrayName;
    request.frameWidth = frameWidth;
    request.frameHeight = frameHeight;
    request.fixedGrid = true;
    request.padding = 1;
    request.encodingMode = converter.m_encodingMode;
    request.monoLayout = converter.m_monoLayout;
    request.codeGenOptions = converter.m_codeGenOptions;
    for (const QVariant &value : urls) {
        const QUrl url = value.toUrl();
        if (url.isValid())
            request.files.append(url);
    }
    const SpriteAtlasResult result = SpriteAtlasService::build(request);
    if (!result.ok) {
        emit converter.errorOccurred(result.errorMessage);
        return false;
    }
    QString path = targetFile.toLocalFile();
    if (path.isEmpty())
        path = targetFile.path();
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        emit converter.errorOccurred(AppLocale::tr("Failed to write atlas: %1").arg(path));
        return false;
    }
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << result.headerCode;
    return true;
}

bool ExportController::saveCodeToFile(DisplayConverter &converter, const QUrl &url)
{
    if (converter.m_generatedCode.isEmpty())
        return false;

    QString path = url.toLocalFile();
    if (path.isEmpty())
        path = url.path();
    if (path.isEmpty()) {
        emit converter.errorOccurred(AppLocale::tr("Specify a file path"));
        return false;
    }

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        emit converter.errorOccurred(AppLocale::tr("Failed to write file: %1").arg(path));
        return false;
    }
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << converter.m_generatedCode;
    converter.rememberExportDir(path);
    if (converter.m_session)
        converter.m_session->addRecentExport(path);
    return true;
}

bool ExportController::saveBinaryToFile(DisplayConverter &converter, const QUrl &url)
{
    if (converter.m_lastResult.width < 1 || converter.m_lastResult.height < 1)
        return false;
    QString path = url.toLocalFile();
    if (path.isEmpty())
        path = url.path();
    if (path.isEmpty()) {
        emit converter.errorOccurred(AppLocale::tr("Specify a file path"));
        return false;
    }

    const QByteArray data = DisplayCodeGenerator::binaryData(
        converter.m_encodingMode,
        converter.m_displayWidth,
        converter.m_displayHeight,
        converter.m_lastResult.monoBits,
        converter.m_lastResult.monoBuffer,
        converter.m_lastResult.grayscale8,
        converter.m_lastResult.rgb565,
        converter.m_lastResult.rgb888,
        converter.m_lastResult.rgb233,
        converter.m_lastResult.rgb24,
        converter.m_monoLayout,
        converter.m_codeGenOptions);

    QString error;
    if (!BinaryExporter::save(path, data, &error)) {
        emit converter.errorOccurred(error);
        return false;
    }
    converter.rememberExportDir(path);
    if (converter.m_session)
        converter.m_session->addRecentExport(path);
    return true;
}

void ExportController::enqueueBatchCodeExport(DisplayConverter &converter,
                                              const QVariantList &urls,
                                              const QUrl &targetFile)
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
        emit converter.errorOccurred(AppLocale::tr("Batch file list is empty"));
        return;
    }
    BatchExportJob job;
    job.files = files;
    job.targetFile = targetFile;
    job.pipeline = converter.pipelineParams();
    job.profileId = converter.m_profileId;
    job.encodingMode = converter.m_encodingMode;
    job.monoLayout = converter.m_monoLayout;
    job.codeGenOptions = converter.m_codeGenOptions;
    converter.m_batchService.enqueue(job);
}
