#include "io/ImageFileReader.h"

#include "io/ImageFormats.h"
#include "translation/AppLocale.h"

#include <QFileInfo>
#include <QImageReader>
#include <QPainter>
#include <QSvgRenderer>

namespace ImageFileReader {

namespace {

QString tooLargeBytesMessage()
{
    return AppLocale::tr("Image is too large (over %1 MB)")
        .arg(kMaxImageBytes / (1024 * 1024));
}

QString tooLargeResolutionMessage(int width, int height)
{
    return AppLocale::tr("Image resolution is too large (%1×%2)")
        .arg(width)
        .arg(height);
}

bool exceedsPixelBudget(int width, int height)
{
    if (width <= 0 || height <= 0)
        return false;
    return qint64(width) * qint64(height) > kMaxImagePixels;
}

QSize fitWithinPixelBudget(QSize size)
{
    if (!size.isValid() || size.width() <= 0 || size.height() <= 0)
        return QSize(512, 512);

    while (exceedsPixelBudget(size.width(), size.height())) {
        const int nextW = qMax(1, size.width() * 3 / 4);
        const int nextH = qMax(1, size.height() * 3 / 4);
        if (nextW == size.width() && nextH == size.height())
            break;
        size = QSize(nextW, nextH);
    }
    return size;
}

bool renderSvg(QSvgRenderer &renderer, QImage *outImage, QString *errorText)
{
    if (!renderer.isValid()) {
        if (errorText)
            *errorText = AppLocale::tr("Failed to load SVG image");
        return false;
    }

    QSize size = renderer.defaultSize();
    if (!size.isValid() || size.width() <= 0 || size.height() <= 0) {
        const QRectF viewBox = renderer.viewBoxF();
        if (viewBox.isValid())
            size = viewBox.size().toSize();
        else
            size = QSize(512, 512);
    }

    size = fitWithinPixelBudget(size);
    if (exceedsPixelBudget(size.width(), size.height())) {
        if (errorText)
            *errorText = tooLargeResolutionMessage(size.width(), size.height());
        return false;
    }

    QImage image(size, QImage::Format_ARGB32);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    renderer.render(&painter);
    painter.end();

    if (image.isNull()) {
        if (errorText)
            *errorText = AppLocale::tr("Failed to rasterize SVG image");
        return false;
    }

    *outImage = image.convertToFormat(QImage::Format_ARGB32);
    return true;
}

bool readWithImageReader(const QString &path, QImage *outImage, QString *errorText)
{
    QImageReader reader(path);
    reader.setAutoTransform(true);
    if (!reader.canRead()) {
        if (errorText)
            *errorText = AppLocale::tr("Failed to load image: %1").arg(path);
        return false;
    }

    const QSize size = reader.size();
    if (size.isValid() && exceedsPixelBudget(size.width(), size.height())) {
        if (errorText)
            *errorText = tooLargeResolutionMessage(size.width(), size.height());
        return false;
    }

    QImage image;
    if (!reader.read(&image) || image.isNull()) {
        if (errorText)
            *errorText = AppLocale::tr("Failed to load image: %1").arg(path);
        return false;
    }

    if (exceedsPixelBudget(image.width(), image.height())) {
        if (errorText)
            *errorText = tooLargeResolutionMessage(image.width(), image.height());
        return false;
    }

    *outImage = image.convertToFormat(QImage::Format_ARGB32);
    return true;
}

bool readWithImageReaderFromData(const QByteArray &data, QImage *outImage, QString *errorText)
{
    QImage image;
    if (!image.loadFromData(data)) {
        if (errorText)
            *errorText = AppLocale::tr("Could not decode image data");
        return false;
    }

    if (exceedsPixelBudget(image.width(), image.height())) {
        if (errorText)
            *errorText = tooLargeResolutionMessage(image.width(), image.height());
        return false;
    }

    *outImage = image.convertToFormat(QImage::Format_ARGB32);
    return true;
}

} // namespace

bool readFromPath(const QString &path, QImage *outImage, QString *errorText)
{
    if (!outImage) {
        if (errorText)
            *errorText = AppLocale::tr("Failed to load image: %1").arg(path);
        return false;
    }

    const QFileInfo info(path);
    if (!info.exists() || !info.isFile()) {
        if (errorText)
            *errorText = AppLocale::tr("Failed to load image: %1").arg(path);
        return false;
    }

    if (info.size() > kMaxImageBytes) {
        if (errorText)
            *errorText = tooLargeBytesMessage();
        return false;
    }

    if (!ImageFormats::isSupportedPath(path)) {
        if (errorText)
            *errorText = AppLocale::tr("Unsupported image format: %1").arg(path);
        return false;
    }

    if (ImageFormats::isSvgPath(path)) {
        QSvgRenderer renderer(path);
        return renderSvg(renderer, outImage, errorText);
    }

    return readWithImageReader(path, outImage, errorText);
}

bool readFromData(const QByteArray &data, const QString &hintPath, QImage *outImage, QString *errorText)
{
    if (!outImage) {
        if (errorText)
            *errorText = AppLocale::tr("Could not decode image data");
        return false;
    }

    if (data.size() > kMaxImageBytes) {
        if (errorText)
            *errorText = tooLargeBytesMessage();
        return false;
    }

    if (ImageFormats::isSvgPath(hintPath)) {
        QSvgRenderer renderer(data);
        return renderSvg(renderer, outImage, errorText);
    }

    if (readWithImageReaderFromData(data, outImage, errorText))
        return true;

    QSvgRenderer renderer(data);
    if (renderer.isValid())
        return renderSvg(renderer, outImage, errorText);

    if (errorText && errorText->isEmpty())
        *errorText = AppLocale::tr("Could not decode image data");
    return false;
}

} // namespace ImageFileReader
