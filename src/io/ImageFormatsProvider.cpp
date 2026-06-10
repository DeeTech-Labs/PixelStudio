#include "io/ImageFormatsProvider.h"

#include "io/ImageFormats.h"

ImageFormatsProvider::ImageFormatsProvider(QObject *parent)
    : QObject(parent)
{
}

QString ImageFormatsProvider::extensionPattern() const
{
    return ImageFormats::extensionPattern();
}
