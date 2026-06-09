#ifndef PIXELSTUDIO_APP_PREVIEWIMAGEPROVIDER_H
#define PIXELSTUDIO_APP_PREVIEWIMAGEPROVIDER_H

#include <QHash>
#include <QImage>
#include <QMutex>
#include <QQuickImageProvider>
#include <QUrl>

class PreviewImageProvider : public QQuickImageProvider
{
public:
    static constexpr const char *kProviderId = "pixelstudio-previews";

    PreviewImageProvider();

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;

    void setImage(const QString &slot, const QImage &image);
    bool hasSlot(const QString &slot) const;
    void clearSlot(const QString &slot);
    void clearTabSlots(const QString &tabId);
    void clearAll();
    QUrl imageUrl(const QString &slot) const;

private:
    mutable QMutex m_mutex;
    QHash<QString, QImage> m_images;
    QHash<QString, quint64> m_epochs;
};

#endif // PIXELSTUDIO_APP_PREVIEWIMAGEPROVIDER_H
