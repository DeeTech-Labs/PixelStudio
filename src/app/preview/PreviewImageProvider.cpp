#include "app/preview/PreviewImageProvider.h"

#include <QMutexLocker>

namespace {

QString slotKeyFromId(const QString &id)
{
    const int lastSlash = id.lastIndexOf(QLatin1Char('/'));
    if (lastSlash <= 0)
        return id;
    return id.left(lastSlash);
}

QImage normalizedImage(const QImage &image)
{
    if (image.isNull())
        return {};
    if (image.format() == QImage::Format_ARGB32 || image.format() == QImage::Format_RGB32)
        return image;
    return image.convertToFormat(QImage::Format_ARGB32);
}

} // namespace

PreviewImageProvider::PreviewImageProvider()
    : QQuickImageProvider(QQuickImageProvider::Image)
{
}

QImage PreviewImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    Q_UNUSED(requestedSize);
    const QString slot = slotKeyFromId(id);
    QMutexLocker lock(&m_mutex);
    const QImage image = m_images.value(slot);
    if (size)
        *size = image.size();
    return image;
}

void PreviewImageProvider::setImage(const QString &slot, const QImage &image)
{
    QMutexLocker lock(&m_mutex);
    if (image.isNull()) {
        m_images.remove(slot);
        m_epochs.remove(slot);
        return;
    }
    const QImage normalized = normalizedImage(image);
    if (m_images.contains(slot) && m_images.value(slot).cacheKey() == normalized.cacheKey())
        return;
    m_images.insert(slot, normalized);
    m_epochs[slot] = m_epochs.value(slot, 0) + 1;
}

bool PreviewImageProvider::hasSlot(const QString &slot) const
{
    QMutexLocker lock(&m_mutex);
    return m_images.contains(slot);
}

void PreviewImageProvider::clearSlot(const QString &slot)
{
    QMutexLocker lock(&m_mutex);
    m_images.remove(slot);
    m_epochs.remove(slot);
}

void PreviewImageProvider::clearTabSlots(const QString &tabId)
{
    if (tabId.isEmpty())
        return;
    const QString prefix = QStringLiteral("tab/") + tabId + QLatin1Char('/');
    QMutexLocker lock(&m_mutex);
    for (auto it = m_images.begin(); it != m_images.end();) {
        if (it.key().startsWith(prefix))
            it = m_images.erase(it);
        else
            ++it;
    }
    for (auto it = m_epochs.begin(); it != m_epochs.end();) {
        if (it.key().startsWith(prefix))
            it = m_epochs.erase(it);
        else
            ++it;
    }
}

void PreviewImageProvider::clearAll()
{
    QMutexLocker lock(&m_mutex);
    m_images.clear();
    m_epochs.clear();
}

QUrl PreviewImageProvider::imageUrl(const QString &slot) const
{
    QMutexLocker lock(&m_mutex);
    if (!m_images.contains(slot))
        return {};
    const quint64 epoch = m_epochs.value(slot);
    return QUrl(QStringLiteral("image://%1/%2/%3")
                    .arg(QLatin1String(kProviderId), slot, QString::number(epoch)));
}
