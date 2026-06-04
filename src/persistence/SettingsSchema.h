#ifndef PIXELSTUDIO_PERSISTENCE_SETTINGSSCHEMA_H
#define PIXELSTUDIO_PERSISTENCE_SETTINGSSCHEMA_H

class QSettings;

namespace SettingsSchema {

inline constexpr int kAppSchemaVersion = 1;
inline constexpr int kSessionSchemaVersion = 1;

void migrateAppSettings(QSettings &settings);
void migrateSessionSettings(QSettings &settings);

} // namespace SettingsSchema

#endif // PIXELSTUDIO_PERSISTENCE_SETTINGSSCHEMA_H
