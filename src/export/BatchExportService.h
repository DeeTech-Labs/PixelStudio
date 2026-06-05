#ifndef BATCHEXPORTSERVICE_H
#define BATCHEXPORTSERVICE_H

#include <QObject>
#include <QQueue>
#include <QUrl>
#include <QVector>
#include <QFutureWatcher>
#include <atomic>

#include "processing/ConvertPipeline.h"
#include "processing/DisplayCodeGenerator.h"

struct BatchExportJob {
    QVector<QUrl> files;
    QUrl targetFile;
    ConvertPipelineParams pipeline;
    QString profileId;
    DisplayCodeGenerator::EncodingMode encodingMode = DisplayCodeGenerator::EncodingMode::Mono1Bit;
    DisplayCodeGenerator::MonoLayout monoLayout = DisplayCodeGenerator::MonoLayout::RowPacked;
    DisplayCodeGenerator::CodeGenOptions codeGenOptions;
};

struct BatchExportResult {
    bool ok = false;
    bool cancelled = false;
    QString code;
    QString errorMessage;
    int progress = 0;
};

class BatchExportService : public QObject
{
    Q_OBJECT
public:
    explicit BatchExportService(QObject *parent = nullptr);

    bool running() const { return m_running; }
    int progress() const { return m_progress; }

    void enqueue(const BatchExportJob &job);
    void cancel();
    void reset();

signals:
    void runningChanged();
    void progressChanged();
    void finished(bool ok, const QString &errorMessage);

private:
    void processQueue();
    void startJob(const BatchExportJob &job);
    void onFinished();

    static BatchExportResult runJob(BatchExportJob job, std::atomic<bool> *cancelFlag);

    QFutureWatcher<BatchExportResult> m_watcher;
    std::atomic<bool> m_cancelled{false};
    bool m_running = false;
    int m_progress = 0;
    QQueue<BatchExportJob> m_queue;
    BatchExportJob m_activeJob;
};

#endif // BATCHEXPORTSERVICE_H
