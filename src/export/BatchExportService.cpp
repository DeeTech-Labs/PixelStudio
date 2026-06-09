#include "export/BatchExportService.h"

#include "LogCategories.h"
#include "translation/AppLocale.h"

#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QtConcurrent>

namespace {

bool loadImageFile(const QUrl &url, QImage &outImage)
{
    QString path = url.toLocalFile();
    if (path.isEmpty())
        path = url.path();
    const QImage img(path);
    if (img.isNull())
        return false;
    outImage = img.convertToFormat(QImage::Format_ARGB32);
    return true;
}

} // namespace

BatchExportService::BatchExportService(QObject *parent)
    : QObject(parent)
{
    connect(&m_watcher, &QFutureWatcher<BatchExportResult>::finished, this, &BatchExportService::onFinished);
}

void BatchExportService::enqueue(const BatchExportJob &job)
{
    m_queue.enqueue(job);
    processQueue();
}

void BatchExportService::cancel()
{
    m_cancelled.store(true, std::memory_order_relaxed);
}

void BatchExportService::reset()
{
    cancel();
    m_queue.clear();
    if (m_running)
        return;

    const bool hadProgress = m_progress != 0;
    m_progress = 0;
    if (hadProgress)
        emit progressChanged();
}

void BatchExportService::processQueue()
{
    if (m_running || m_queue.isEmpty())
        return;
    startJob(m_queue.dequeue());
}

void BatchExportService::startJob(const BatchExportJob &job)
{
    m_activeJob = job;
    m_cancelled.store(false, std::memory_order_relaxed);
    m_running = true;
    m_progress = 0;
    emit runningChanged();
    emit progressChanged();

    std::atomic<bool> *cancelPtr = &m_cancelled;
    auto future = QtConcurrent::run([job, cancelPtr]() { return runJob(job, cancelPtr); });
    m_watcher.setFuture(future);
}

BatchExportResult BatchExportService::runJob(BatchExportJob job, std::atomic<bool> *cancelFlag)
{
    BatchExportResult result;
    if (job.files.isEmpty()) {
        result.errorMessage = AppLocale::tr("Batch file list is empty");
        return result;
    }

    QStringList arrayNames;
    QString output;

    for (int i = 0; i < job.files.size(); ++i) {
        if (cancelFlag && cancelFlag->load(std::memory_order_relaxed)) {
            result.cancelled = true;
            result.errorMessage = AppLocale::tr("Batch export cancelled");
            return result;
        }

        QImage image;
        if (!loadImageFile(job.files[i], image)) {
            result.errorMessage = AppLocale::tr("Failed to load: %1").arg(job.files[i].toString());
            return result;
        }

        const QImage oriented = ConvertPipeline::applyOrientation(image, job.pipeline);
        auto raster = ConvertPipeline::rasterize(oriented, job.pipeline);
        if (raster.preview.isNull()) {
            result.errorMessage = AppLocale::tr("Conversion failed: %1").arg(job.files[i].toString());
            return result;
        }

        const QString fileStem = QFileInfo(job.files[i].toLocalFile()).baseName();
        const QString name = DisplayCodeGenerator::sanitizeIdentifier(
            QStringLiteral("%1_%2").arg(fileStem, QString::number(i)));
        arrayNames << name;

        DisplayProfile profile = DisplayProfile::byId(job.profileId);
        profile.width = job.pipeline.displayWidth;
        profile.height = job.pipeline.displayHeight;
        profile.colorMode = job.pipeline.colorMode;

        output += DisplayCodeGenerator::generate(profile,
                                                 job.pipeline.displayWidth,
                                                 job.pipeline.displayHeight,
                                                 job.encodingMode,
                                                 raster.monoBits,
                                                 raster.monoBuffer,
                                                 raster.grayscale8,
                                                 raster.rgb565,
                                                 raster.rgb888,
                                                 raster.rgb233,
                                                 raster.rgb24,
                                                 name,
                                                 job.monoLayout,
                                                 job.codeGenOptions,
                                                 raster.indexedPalette);
        output += QLatin1Char('\n');

        result.progress = qRound((qreal(i + 1) / qreal(job.files.size())) * 100.0);
    }

    output += QStringLiteral("const void* batch_images[] = {\n");
    for (int i = 0; i < arrayNames.size(); ++i) {
        output += QStringLiteral("    %1").arg(arrayNames[i]);
        if (i < arrayNames.size() - 1)
            output += QStringLiteral(",");
        output += QLatin1Char('\n');
    }
    output += QStringLiteral("};\n");

    result.ok = true;
    result.code = output;
    return result;
}

void BatchExportService::onFinished()
{
    const BatchExportResult result = m_watcher.result();
    m_progress = result.progress;
    emit progressChanged();

    bool writeOk = false;
    QString error = result.errorMessage;

    if (result.cancelled) {
        writeOk = false;
    } else if (result.ok) {
        QString path = m_activeJob.targetFile.toLocalFile();
        if (path.isEmpty())
            path = m_activeJob.targetFile.path();
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
            error = AppLocale::tr("Failed to write batch file: %1").arg(path);
        } else {
            QTextStream out(&file);
            out.setEncoding(QStringConverter::Utf8);
            out << result.code;
            writeOk = true;
        }
    }

    m_running = false;
    emit runningChanged();
    const QString finishedError = error.isEmpty() && !writeOk ? AppLocale::tr("Batch conversion failed") : error;
    if (!writeOk && !finishedError.isEmpty())
        qCWarning(lcExport) << finishedError;
    emit finished(writeOk, finishedError);

    processQueue();
}
