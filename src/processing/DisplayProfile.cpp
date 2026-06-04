#include "processing/DisplayProfile.h"

QVector<DisplayProfile> DisplayProfile::presets()
{
    return {
        {QStringLiteral("128x64"), QStringLiteral("128×64"), 128, 64},
        {QStringLiteral("128x32"), QStringLiteral("128×32"), 128, 32},
        {QStringLiteral("72x40"), QStringLiteral("72×40"), 72, 40},
        {QStringLiteral("64x48"), QStringLiteral("64×48"), 64, 48},
        {QStringLiteral("128x128"), QStringLiteral("128×128"), 128, 128},
        {QStringLiteral("250x122"), QStringLiteral("250×122"), 250, 122},
        {QStringLiteral("128x160"), QStringLiteral("128×160"), 128, 160},
        {QStringLiteral("240x240"), QStringLiteral("240×240"), 240, 240},
        {QStringLiteral("135x240"), QStringLiteral("135×240"), 135, 240},
        {QStringLiteral("320x240"), QStringLiteral("320×240"), 320, 240},
        {QStringLiteral("170x320"), QStringLiteral("170×320"), 170, 320},
        {QStringLiteral("480x320"), QStringLiteral("480×320"), 480, 320},
        {QStringLiteral("custom"), QStringLiteral("Custom size"), 128, 64},
    };
}

DisplayProfile DisplayProfile::byId(const QString &id)
{
    for (const DisplayProfile &p : presets()) {
        if (p.id == id)
            return p;
    }
    return presets().constFirst();
}
