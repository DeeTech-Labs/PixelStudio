#include "io/ImageLoader.h"

#include "LogCategories.h"
#include "translation/AppLocale.h"

#include <QClipboard>
#include <QFileInfo>
#include <QGuiApplication>
#include <QImageReader>
#include <QMimeData>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QtConcurrent>
#include <QVariant>

namespace {
constexpr qint64 kMaxImageBytes = 20 * 1024 * 1024;
constexpr int kMaxImagePixels = 4096 * 4096;
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

QString resolveLocalPath(const QUrl &url)
{
    QString path = url.toLocalFile();
    if (path.isEmpty())
        path = url.path();
    return path;
}

} // namespace

ImageLoader::ImageLoader(QObject *parent)
    : QObject{parent}
{
    connect(&m_fileLoadWatcher, &QFutureWatcher<FileLoadOutcome>::finished, this, &ImageLoader::onFileLoadFinished);
}

void ImageLoader::setLoading(bool loading)
{
    if (m_loading == loading)
        return;
    m_loading = loading;
    emit loadingChanged();
}

void ImageLoader::emitLoadError(const QString &message)
{
    qCWarning(lcIo) << message;
    emit error(message);
}

FileLoadOutcome ImageLoader::loadFileWorker(const QUrl &url)
{
    FileLoadOutcome outcome;
    outcome.sourceUrl = url;

    const QString path = resolveLocalPath(url);
    if (path.isEmpty()) {
        outcome.errorMessage = AppLocale::tr("Failed to load image: %1").arg(url.toString());
        return outcome;
    }

    const QFileInfo info(path);
    if (!info.exists() || !info.isFile()) {
        outcome.errorMessage = AppLocale::tr("Failed to load image: %1").arg(path);
        return outcome;
    }

    if (info.size() > kMaxImageBytes) {
        outcome.errorMessage = AppLocale::tr("Image is too large (over %1 MB)")
                                   .arg(kMaxImageBytes / (1024 * 1024));
        return outcome;
    }

    QImageReader reader(path);
    reader.setAutoTransform(true);
    if (!reader.canRead()) {
        outcome.errorMessage = AppLocale::tr("Failed to load image: %1").arg(path);
        return outcome;
    }

    const QSize size = reader.size();
    if (size.isValid() && size.width() > 0 && size.height() > 0) {
        const qint64 pixels = qint64(size.width()) * qint64(size.height());
        if (pixels > kMaxImagePixels) {
            outcome.errorMessage = AppLocale::tr("Image resolution is too large (%1×%2)")
                                       .arg(size.width())
                                       .arg(size.height());
            return outcome;
        }
    }

    QImage img;
    if (!reader.read(&img) || img.isNull()) {
        outcome.errorMessage = AppLocale::tr("Failed to load image: %1").arg(path);
        return outcome;
    }

    outcome.ok = true;
    outcome.image = img.convertToFormat(QImage::Format_ARGB32);
    return outcome;
}

bool ImageLoader::loadFromFile(const QUrl &url, QImage &outImage)
{
    const FileLoadOutcome outcome = loadFileWorker(url);
    if (!outcome.ok) {
        emitLoadError(outcome.errorMessage);
        return false;
    }
    outImage = outcome.image;
    return true;
}

void ImageLoader::loadFromFileAsync(const QUrl &url)
{
    if (m_fileLoadWatcher.isRunning())
        m_fileLoadWatcher.cancel();

    setLoading(true);
    m_fileLoadWatcher.setFuture(QtConcurrent::run(loadFileWorker, url));
}

void ImageLoader::onFileLoadFinished()
{
    setLoading(false);
    const FileLoadOutcome outcome = m_fileLoadWatcher.result();
    if (!outcome.ok) {
        emitLoadError(outcome.errorMessage);
        return;
    }
    emit loaded(outcome.image, outcome.sourceUrl);
}

bool ImageLoader::loadFromClipboard(QImage &outImage)
{
    QClipboard *clipboard = QGuiApplication::clipboard();
    if (!clipboard) {
        emitLoadError(AppLocale::tr("No image in clipboard"));
        return false;
    }

    const QMimeData *mime = clipboard->mimeData(QClipboard::Clipboard);
    const QImage img = imageFromMimeData(mime);
    if (img.isNull()) {
        emitLoadError(AppLocale::tr("No image in clipboard"));
        return false;
    }

    const qint64 pixels = qint64(img.width()) * qint64(img.height());
    if (pixels > kMaxImagePixels) {
        emitLoadError(AppLocale::tr("Image resolution is too large (%1×%2)")
                       .arg(img.width())
                       .arg(img.height()));
        return false;
    }

    outImage = img.convertToFormat(QImage::Format_ARGB32);
    return true;
}

void ImageLoader::loadFromUrl(const QString &urlString)
{
    QUrl url(urlString.trimmed());
    if (!url.isValid() || (!url.scheme().startsWith("http", Qt::CaseInsensitive) && url.scheme() != "file")) {
        emitLoadError(AppLocale::tr("Invalid URL: %1").arg(urlString));
        return;
    }
    if (url.scheme() == "file") {
        loadFromFileAsync(url);
        return;
    }
    setLoading(true);
    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::NoLessSafeRedirectPolicy);
    request.setTransferTimeout(kNetworkTimeoutMs);
    request.setRawHeader("Accept", "image/*");
    QNetworkReply *reply = m_networkManager.get(request);
    connect(reply, &QNetworkReply::downloadProgress, this, [this, reply](qint64 received, qint64) {
        if (received > kMaxImageBytes) {
            reply->setProperty("tooLarge", true);
            reply->abort();
            setLoading(false);
            emitLoadError(AppLocale::tr("Image is too large (over %1 MB)")
                           .arg(kMaxImageBytes / (1024 * 1024)));
        }
    });
    connect(reply, &QNetworkReply::finished, this, &ImageLoader::onUrlDownloadFinished);
}

void ImageLoader::onUrlDownloadFinished()
{
    setLoading(false);
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply)
        return;
    reply->deleteLater();

    if (reply->property("tooLarge").toBool())
        return;

    if (reply->error() != QNetworkReply::NoError) {
        emitLoadError(AppLocale::tr("Download error: %1").arg(reply->errorString()));
        return;
    }

    const QString contentType = reply->header(QNetworkRequest::ContentTypeHeader).toString();
    if (!contentType.isEmpty() && !contentType.startsWith(QStringLiteral("image/"), Qt::CaseInsensitive)) {
        emitLoadError(AppLocale::tr("URL is not an image (Content-Type: %1)").arg(contentType));
        return;
    }

    QByteArray data = reply->readAll();
    if (data.size() > kMaxImageBytes) {
        emitLoadError(AppLocale::tr("Image is too large (over %1 MB)")
                       .arg(kMaxImageBytes / (1024 * 1024)));
        return;
    }
    QImage img;
    if (!img.loadFromData(data)) {
        emitLoadError(AppLocale::tr("Could not decode image from URL"));
        return;
    }

    const qint64 pixels = qint64(img.width()) * qint64(img.height());
    if (pixels > kMaxImagePixels) {
        emitLoadError(AppLocale::tr("Image resolution is too large (%1×%2)")
                       .arg(img.width())
                       .arg(img.height()));
        return;
    }

    emit loaded(img.convertToFormat(QImage::Format_ARGB32), reply->url());
}
