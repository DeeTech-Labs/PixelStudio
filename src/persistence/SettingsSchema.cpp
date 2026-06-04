#include "persistence/SettingsSchema.h"

#include <QSettings>

namespace SettingsSchema {

void migrateAppSettings(QSettings &settings)
{
    const int version = settings.value(QStringLiteral("app/schemaVersion"), 0).toInt();
    if (version >= kAppSchemaVersion) {
        settings.setValue(QStringLiteral("app/schemaVersion"), kAppSchemaVersion);
        return;
    }
    settings.setValue(QStringLiteral("app/schemaVersion"), kAppSchemaVersion);
}

void migrateSessionSettings(QSettings &settings)
{
    const int version = settings.value(QStringLiteral("session/schemaVersion"), 0).toInt();
    if (version >= kSessionSchemaVersion) {
        settings.setValue(QStringLiteral("session/schemaVersion"), kSessionSchemaVersion);
        return;
    }
    settings.setValue(QStringLiteral("session/schemaVersion"), kSessionSchemaVersion);
}

} // namespace SettingsSchema
