#include "persistence/ProjectFormatIo.h"

namespace ProjectFormatIo {

void appendU8(QByteArray &out, quint8 v)
{
    out.append(char(v));
}

void appendU16(QByteArray &out, quint16 v)
{
    out.append(char(v & 0xff));
    out.append(char((v >> 8) & 0xff));
}

void appendS16(QByteArray &out, qint16 v)
{
    appendU16(out, quint16(v));
}

void appendS32(QByteArray &out, qint32 v)
{
    appendU32(out, quint32(v));
}

void appendU32(QByteArray &out, quint32 v)
{
    out.append(char(v & 0xff));
    out.append(char((v >> 8) & 0xff));
    out.append(char((v >> 16) & 0xff));
    out.append(char((v >> 24) & 0xff));
}

void appendString(QByteArray &out, const QString &s)
{
    const QByteArray utf8 = s.toUtf8();
    const quint16 len = quint16(qMin(utf8.size(), 0xffff));
    appendU16(out, len);
    out.append(utf8.constData(), len);
}

void appendBlob(QByteArray &out, const QByteArray &blob)
{
    const quint32 len = quint32(qMin(blob.size(), 0x7fffffff));
    appendU32(out, len);
    if (len > 0)
        out.append(blob.constData(), len);
}

bool readU8(const QByteArray &in, int &pos, quint8 *v)
{
    if (pos >= in.size())
        return false;
    *v = quint8(uchar(in.at(pos++)));
    return true;
}

bool readU16(const QByteArray &in, int &pos, quint16 *v)
{
    if (pos + 2 > in.size())
        return false;
    const auto b = reinterpret_cast<const uchar *>(in.constData() + pos);
    *v = quint16(b[0]) | (quint16(b[1]) << 8);
    pos += 2;
    return true;
}

bool readS16(const QByteArray &in, int &pos, qint16 *v)
{
    quint16 u = 0;
    if (!readU16(in, pos, &u))
        return false;
    *v = qint16(u);
    return true;
}

bool readS32(const QByteArray &in, int &pos, qint32 *v)
{
    quint32 u = 0;
    if (!readU32(in, pos, &u))
        return false;
    *v = qint32(u);
    return true;
}

bool readU32(const QByteArray &in, int &pos, quint32 *v)
{
    if (pos + 4 > in.size())
        return false;
    const auto b = reinterpret_cast<const uchar *>(in.constData() + pos);
    *v = quint32(b[0]) | (quint32(b[1]) << 8) | (quint32(b[2]) << 16) | (quint32(b[3]) << 24);
    pos += 4;
    return true;
}

bool readString(const QByteArray &in, int &pos, QString *s)
{
    quint16 len = 0;
    if (!readU16(in, pos, &len))
        return false;
    if (pos + len > in.size())
        return false;
    *s = QString::fromUtf8(in.constData() + pos, len);
    pos += len;
    return true;
}

bool readBlob(const QByteArray &in, int &pos, QByteArray *blob)
{
    if (!blob)
        return false;
    blob->clear();
    quint32 len = 0;
    if (!readU32(in, pos, &len))
        return false;
    if (len == 0)
        return true;
    if (pos + int(len) > in.size())
        return false;
    *blob = in.mid(pos, int(len));
    pos += int(len);
    return true;
}

bool ensureRemaining(const QByteArray &in, int pos, int bytes, QString *errorText, const char *context)
{
    if (pos + bytes <= in.size())
        return true;
    if (errorText)
        *errorText = QStringLiteral("Truncated project data at %1").arg(QString::fromLatin1(context));
    return false;
}

} // namespace ProjectFormatIo
