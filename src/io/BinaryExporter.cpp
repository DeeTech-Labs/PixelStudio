#include "io/BinaryExporter.h"

#include "translation/AppLocale.h"

#include <QFile>

bool BinaryExporter::save(const QString &path, const QByteArray &data, QString *errorText)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (errorText)
            *errorText = AppLocale::tr("Failed to open file: %1").arg(path);
        return false;
    }
    const qint64 written = file.write(data);
    if (written != data.size()) {
        if (errorText)
            *errorText = AppLocale::tr("Failed to write file: %1").arg(path);
        return false;
    }
    return true;
}
