#include "app/converter/ExportController.h"
#include "app/DisplayConverter.h"

#include "export/BatchExportService.h"
#include "export/SpriteAtlasService.h"
#include "translation/AppLocale.h"
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
    ConverterState &state = converter.converterState();
    QString out = outputFolder.trimmed();
    if (out.isEmpty())
        out = AppPaths::watchDir();
    state.uiState.watchInputFolder = inputFolder;
    state.uiState.watchOutputFolder = out;
    converter.watchService()->configure(inputFolder, out);
    converter.updateWatchExportPrefix();
    converter.schedulePersistSession();
}

void ExportController::setWatchFolderActive(DisplayConverter &converter, bool active)
{
    converter.converterState().uiState.watchActive = active;
    converter.watchService()->setActive(active);
    converter.schedulePersistSession();
}

bool ExportController::buildSpriteAtlas(DisplayConverter &converter,
                                        const QVariantList &urls,
                                        const QUrl &targetFile,
                                        int frameWidth,
                                        int frameHeight)
{
    const ConverterState &state = converter.converterState();
    SpriteAtlasRequest request;
    request.pipeline = converter.pipelineParams();
    request.arrayPrefix = state.arrayName.isEmpty() ? QStringLiteral("sprite") : state.arrayName;
    request.frameWidth = frameWidth;
    request.frameHeight = frameHeight;
    request.fixedGrid = true;
    request.padding = 1;
    request.encodingMode = state.encodingMode;
    request.monoLayout = state.monoLayout;
    request.codeGenOptions = state.codeGenOptions;
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
    const ConverterState &state = converter.converterState();
    if (state.generatedCode.isEmpty())
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
    out << state.generatedCode;
    converter.rememberExportDir(path);
    if (converter.session())
        converter.session()->addRecentExport(path);
    return true;
}

bool ExportController::saveBinaryToFile(DisplayConverter &converter, const QUrl &url)
{
    const ConverterState &state = converter.converterState();
    if (state.lastResult.width < 1 || state.lastResult.height < 1)
        return false;
    QString path = url.toLocalFile();
    if (path.isEmpty())
        path = url.path();
    if (path.isEmpty()) {
        emit converter.errorOccurred(AppLocale::tr("Specify a file path"));
        return false;
    }

    const QByteArray data = DisplayCodeGenerator::binaryData(
        state.encodingMode,
        state.displayWidth,
        state.displayHeight,
        state.lastResult.monoBits,
        state.lastResult.monoBuffer,
        state.lastResult.grayscale8,
        state.lastResult.rgb565,
        state.lastResult.rgb888,
        state.lastResult.rgb233,
        state.lastResult.rgb24,
        state.monoLayout,
        state.codeGenOptions);

    QString error;
    if (!BinaryExporter::save(path, data, &error)) {
        emit converter.errorOccurred(error);
        return false;
    }
    converter.rememberExportDir(path);
    if (converter.session())
        converter.session()->addRecentExport(path);
    return true;
}

void ExportController::enqueueBatchCodeExport(DisplayConverter &converter,
                                              const QVariantList &urls,
                                              const QUrl &targetFile)
{
    const ConverterState &state = converter.converterState();
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
    job.profileId = state.profileId;
    job.encodingMode = state.encodingMode;
    job.monoLayout = state.monoLayout;
    job.codeGenOptions = state.codeGenOptions;
    converter.batchService()->enqueue(job);
}
