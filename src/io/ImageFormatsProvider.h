#ifndef PIXELSTUDIO_IO_IMAGEFORMATSPROVIDER_H
#define PIXELSTUDIO_IO_IMAGEFORMATSPROVIDER_H

#include <QObject>
#include <QString>

class ImageFormatsProvider : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString extensionPattern READ extensionPattern CONSTANT)

public:
    explicit ImageFormatsProvider(QObject *parent = nullptr);

    QString extensionPattern() const;
};

#endif // PIXELSTUDIO_IO_IMAGEFORMATSPROVIDER_H
