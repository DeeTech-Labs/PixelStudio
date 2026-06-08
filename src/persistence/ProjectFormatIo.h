#ifndef PIXELSTUDIO_PERSISTENCE_PROJECTFORMATIO_H
#define PIXELSTUDIO_PERSISTENCE_PROJECTFORMATIO_H

#include <QByteArray>
#include <QString>

namespace ProjectFormatIo {

void appendU8(QByteArray &out, quint8 v);
void appendU16(QByteArray &out, quint16 v);
void appendS16(QByteArray &out, qint16 v);
void appendU32(QByteArray &out, quint32 v);
void appendString(QByteArray &out, const QString &s);
void appendBlob(QByteArray &out, const QByteArray &blob);

bool readU8(const QByteArray &in, int &pos, quint8 *v);
bool readU16(const QByteArray &in, int &pos, quint16 *v);
bool readS16(const QByteArray &in, int &pos, qint16 *v);
bool readU32(const QByteArray &in, int &pos, quint32 *v);
bool readString(const QByteArray &in, int &pos, QString *s);
bool readBlob(const QByteArray &in, int &pos, QByteArray *blob);

bool ensureRemaining(const QByteArray &in, int pos, int bytes, QString *errorText, const char *context);

} // namespace ProjectFormatIo

#endif // PIXELSTUDIO_PERSISTENCE_PROJECTFORMATIO_H
