#include "i18n/AppLocale.h"
#include "io/ImageLoader.h"
#include <QClipboard>
#include <QGuiApplication>
#include <QMimeData>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QVariant>

namespace {
constexpr qint64 kMaxDownloadedImageBytes = 20 * 1024 * 1024;
constexpr int kNetworkTimeoutMs = 12000;

struct ClipboardImageFormat {
    const char *mime;
    const char *reader;
};

constexpr ClipboardImageFormat kClipboardImageFormats[] = {
    { "image/png", "PNG" },
    { "image/jpeg", "JPEG" },
    { "image/bmp", "BMP" },
    { "image/webp", "WEBP" },
    { "image/gif", "GIF" },
};

QImage imageFromMimeData(const QMimeData *mime)
{
    if (!mime)
        return {};

    if (mime->hasImage()) {
        const QImage img = qvariant_cast<QImage>(mime->imageData());
        if (!img.isNull())
            return img;
    }

    for (const ClipboardImageFormat &entry : kClipboardImageFormats) {
        if (!mime->hasFormat(entry.mime))
            continue;
        QImage img;
        if (img.loadFromData(mime->data(entry.mime), entry.reader))
            return img;
    }

    return {};
}
}

ImageLoader::ImageLoader(QObject *parent)
    : QObject{parent}
{
}

bool ImageLoader::loadFromFile(const QUrl &url, QImage &outImage)
{
    QString path = url.toLocalFile();
    if (path.isEmpty())
        path = url.path();

    QImage img(path);
    if (img.isNull()) {
        emit error(AppLocale::tr("Failed to load image: %1").arg(path));
        return false;
    }
    outImage = img.convertToFormat(QImage::Format_ARGB32);
    return true;
}

bool ImageLoader::loadFromClipboard(QImage &outImage)
{
    QClipboard *clipboard = QGuiApplication::clipboard();
    if (!clipboard) {
        emit error(AppLocale::tr("No image in clipboard"));
        return false;
    }

    // Single clipboard read — multiple image()/pixmap()/mimeData() calls on Windows
    // trigger qt.qpa.mime "Retrying to obtain clipboard" noise.
    const QMimeData *mime = clipboard->mimeData(QClipboard::Clipboard);
    const QImage img = imageFromMimeData(mime);
    if (img.isNull()) {
        emit error(AppLocale::tr("No image in clipboard"));
        return false;
    }
    outImage = img.convertToFormat(QImage::Format_ARGB32);
    return true;
}

void ImageLoader::loadFromUrl(const QString &urlString)
{
    QUrl url(urlString.trimmed());
    if (!url.isValid() || (!url.scheme().startsWith("http", Qt::CaseInsensitive) && url.scheme() != "file")) {
        emit error(AppLocale::tr("Invalid URL: %1").arg(urlString));
        return;
    }
    if (url.scheme() == "file") {
        QImage img;
        if (loadFromFile(url, img))
            emit loaded(img, url);
        return;
    }
    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::NoLessSafeRedirectPolicy);
    request.setTransferTimeout(kNetworkTimeoutMs);
    request.setRawHeader("Accept", "image/*");
    QNetworkReply *reply = m_networkManager.get(request);
    connect(reply, &QNetworkReply::downloadProgress, this, [this, reply](qint64 received, qint64) {
        if (received > kMaxDownloadedImageBytes) {
            reply->setProperty("tooLarge", true);
            reply->abort();
            emit error(AppLocale::tr("Image is too large (over %1 MB)")
                           .arg(kMaxDownloadedImageBytes / (1024 * 1024)));
        }
    });
    connect(reply, &QNetworkReply::finished, this, &ImageLoader::onUrlDownloadFinished);
}

void ImageLoader::onUrlDownloadFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply) return;
    reply->deleteLater();

    if (reply->property("tooLarge").toBool())
        return;

    if (reply->error() != QNetworkReply::NoError) {
        emit error(AppLocale::tr("Download error: %1").arg(reply->errorString()));
        return;
    }

    const QString contentType = reply->header(QNetworkRequest::ContentTypeHeader).toString();
    if (!contentType.isEmpty() && !contentType.startsWith(QStringLiteral("image/"), Qt::CaseInsensitive)) {
        emit error(AppLocale::tr("URL is not an image (Content-Type: %1)").arg(contentType));
        return;
    }

    QByteArray data = reply->readAll();
    if (data.size() > kMaxDownloadedImageBytes) {
        emit error(AppLocale::tr("Image is too large (over %1 MB)")
                       .arg(kMaxDownloadedImageBytes / (1024 * 1024)));
        return;
    }
    QImage img;
    if (!img.loadFromData(data)) {
        emit error(AppLocale::tr("Could not decode image from URL"));
        return;
    }
    emit loaded(img.convertToFormat(QImage::Format_ARGB32), reply->url());
}
