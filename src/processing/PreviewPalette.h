#ifndef PIXELSTUDIO_PROCESSING_PREVIEWPALETTE_H
#define PIXELSTUDIO_PROCESSING_PREVIEWPALETTE_H

#include <QImage>
#include <QVariantList>
#include <QVector>

namespace PreviewPalette {

QVariantList fromRgbList(const QVector<quint32> &colors);
QVariantList fromPreviewImage(const QImage &img, int maxColors = 256);

} // namespace PreviewPalette

#endif // PIXELSTUDIO_PROCESSING_PREVIEWPALETTE_H
