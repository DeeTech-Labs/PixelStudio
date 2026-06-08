#ifndef PIXELSTUDIO_PERSISTENCE_PROJECTFORMATDECODER_H
#define PIXELSTUDIO_PERSISTENCE_PROJECTFORMATDECODER_H

#include <QByteArray>
#include <QString>

struct StudioProject;

namespace ProjectFormatDecoder {

bool decodeFile(const QByteArray &fileData, StudioProject *project, QString *errorText = nullptr);

} // namespace ProjectFormatDecoder

#endif // PIXELSTUDIO_PERSISTENCE_PROJECTFORMATDECODER_H
