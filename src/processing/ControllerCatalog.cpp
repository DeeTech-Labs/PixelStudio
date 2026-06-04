#include "processing/ControllerCatalog.h"

#include <QVariantMap>

QVariantList ControllerCatalog::workflowPresets()
{
    return {
        QVariantMap{{QStringLiteral("id"), QStringLiteral("icon")},
                    {QStringLiteral("name"), QStringLiteral("Icon mode")},
                    {QStringLiteral("description"), QStringLiteral("Mono crop + packed/RLE for 16/24/32px icons")}},
        QVariantMap{{QStringLiteral("id"), QStringLiteral("splash")},
                    {QStringLiteral("name"), QStringLiteral("Splash mode")},
                    {QStringLiteral("description"), QStringLiteral("RGB565 splash for small TFT displays")}},
        QVariantMap{{QStringLiteral("id"), QStringLiteral("epaper")},
                    {QStringLiteral("name"), QStringLiteral("E-paper mode")},
                    {QStringLiteral("description"), QStringLiteral("High contrast mono with RLE packing")}},
        QVariantMap{{QStringLiteral("id"), QStringLiteral("indexed")},
                    {QStringLiteral("name"), QStringLiteral("Indexed palette mode")},
                    {QStringLiteral("description"), QStringLiteral("Posterized RGB233-style palette workflow")}},
    };
}
