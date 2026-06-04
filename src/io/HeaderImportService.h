#ifndef PIXELSTUDIO_IO_HEADERIMPORTSERVICE_H
#define PIXELSTUDIO_IO_HEADERIMPORTSERVICE_H

#include <QImage>
#include <QUrl>

#include "processing/DisplayCodeGenerator.h"
#include "processing/DisplayProfile.h"

struct HeaderImportResult
{
    bool ok = false;
    QString errorMessage;
    QString arrayName;
    int width = 0;
    int height = 0;
    DisplayProfile::ColorMode colorMode = DisplayProfile::Mono1Bit;
    DisplayCodeGenerator::EncodingMode encodingMode = DisplayCodeGenerator::EncodingMode::Mono8HorizontalMsb;
    QImage preview;
};

class HeaderImportService
{
public:
    static HeaderImportResult importHeader(const QUrl &url);
};

#endif // PIXELSTUDIO_IO_HEADERIMPORTSERVICE_H
