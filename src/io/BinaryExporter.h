#ifndef BINARYEXPORTER_H
#define BINARYEXPORTER_H

#include <QByteArray>
#include <QString>

class BinaryExporter
{
public:
    static bool save(const QString &path, const QByteArray &data, QString *errorText = nullptr);
};

#endif // BINARYEXPORTER_H
