#ifndef PIXELSTUDIO_PERSISTENCE_PROJECTFORMATENCODER_H
#define PIXELSTUDIO_PERSISTENCE_PROJECTFORMATENCODER_H

#include <QByteArray>

struct StudioProject;

namespace ProjectFormatEncoder {

QByteArray buildPayload(const StudioProject &project);
QByteArray encodeFile(const StudioProject &project);

} // namespace ProjectFormatEncoder

#endif // PIXELSTUDIO_PERSISTENCE_PROJECTFORMATENCODER_H
