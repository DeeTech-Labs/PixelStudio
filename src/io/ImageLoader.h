#ifndef IMAGELOADER_H
#define IMAGELOADER_H

#include <QObject>
#include <QImage>
#include <QUrl>
#include <QNetworkAccessManager>

class ImageLoader : public QObject
{
    Q_OBJECT
public:
    explicit ImageLoader(QObject *parent = nullptr);

    bool loadFromFile(const QUrl &url, QImage &outImage);
    bool loadFromClipboard(QImage &outImage);
    void loadFromUrl(const QString &urlString);

signals:
    void loaded(const QImage &image, const QUrl &sourceUrl);
    void error(const QString &message);

private Q_SLOTS:
    void onUrlDownloadFinished();

private:
    QNetworkAccessManager m_networkManager;
};

#endif // IMAGELOADER_H
