#ifndef IMAGELOADER_H
#define IMAGELOADER_H

#include <QObject>
#include <QImage>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QFutureWatcher>

struct FileLoadOutcome
{
    bool ok = false;
    QImage image;
    QUrl sourceUrl;
    QString errorMessage;
};

class ImageLoader : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)
public:
    explicit ImageLoader(QObject *parent = nullptr);

    bool loading() const { return m_loading; }

    bool loadFromFile(const QUrl &url, QImage &outImage);
    void loadFromFileAsync(const QUrl &url);
    bool loadFromClipboard(QImage &outImage);
    void loadFromUrl(const QString &urlString);

signals:
    void loaded(const QImage &image, const QUrl &sourceUrl);
    void error(const QString &message);
    void loadingChanged();

private Q_SLOTS:
    void onUrlDownloadFinished();
    void onFileLoadFinished();

private:
    static FileLoadOutcome loadFileWorker(const QUrl &url);
    void setLoading(bool loading);

    QNetworkAccessManager m_networkManager;
    QFutureWatcher<FileLoadOutcome> m_fileLoadWatcher;
    bool m_loading = false;
};

#endif // IMAGELOADER_H
