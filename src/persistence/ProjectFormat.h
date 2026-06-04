#ifndef PIXELSTUDIO_PERSISTENCE_PROJECTFORMAT_H
#define PIXELSTUDIO_PERSISTENCE_PROJECTFORMAT_H

#include <QByteArray>
#include <QString>

struct StudioProject;

namespace ProjectFormat {

inline constexpr char kMagic[4] = {'P', 'S', 'P', 'X'};
inline constexpr quint8 kVersion = 2;

QString extension();

bool isNativePayload(const QByteArray &head);
bool isProjectPath(const QString &path);

QByteArray encode(const StudioProject &project);
bool decode(const QByteArray &fileData, StudioProject *project, QString *errorText = nullptr);

} // namespace ProjectFormat

#endif // PIXELSTUDIO_PERSISTENCE_PROJECTFORMAT_H
