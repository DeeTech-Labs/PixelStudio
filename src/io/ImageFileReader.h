#ifndef PIXELSTUDIO_IO_IMAGEFILEREADER_H
#define PIXELSTUDIO_IO_IMAGEFILEREADER_H

#include <QByteArray>
#include <QImage>
#include <QString>

namespace ImageFileReader {

constexpr qint64 kMaxImageBytes = 20 * 1024 * 1024;
constexpr int kMaxImagePixels = 4096 * 4096;

bool readFromPath(const QString &path, QImage *outImage, QString *errorText = nullptr);
bool readFromData(const QByteArray &data, const QString &hintPath, QImage *outImage, QString *errorText = nullptr);

} // namespace ImageFileReader

#endif // PIXELSTUDIO_IO_IMAGEFILEREADER_H
