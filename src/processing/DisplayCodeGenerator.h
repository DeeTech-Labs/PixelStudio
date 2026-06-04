#ifndef DISPLAYCODEGENERATOR_H
#define DISPLAYCODEGENERATOR_H

#include <QString>
#include <QByteArray>
#include <QVector>
#include <QVariantList>

#include "processing/DisplayProfile.h"

class DisplayCodeGenerator
{
public:
    enum class EncodingMode {
        Mono1PixPerByte = 0,
        Mono8HorizontalLsb = 1,
        Mono8HorizontalMsb = 2,
        Mono8VerticalCol = 3,
        Mono8VerticalRow = 4,
        PackedImageAuto = 5,
        PackedImageHeader = 6,
        PackedImageRle = 7,
        Grayscale8 = 8,
        Rgb24 = 9,
        Rgb888 = 10,
        Rgb565 = 11,
        Rgb233 = 12,
        Ascii = 13,
        Bricks = 14
    };

    enum class MonoLayout {
        RowPacked = 0,
        Ssd1306Page = 1
    };

    struct CodeGenOptions {
        bool includeHeaderComments = true;
        bool useProgmem = true;
        bool staticStorage = true;
    };

    static QVariantList availableEncodings();
    static QString extractArrayBody(const QString &fullCode);
    static QString sanitizeIdentifier(QString name);

    static QByteArray binaryData(EncodingMode mode,
                                 int width,
                                 int height,
                                 const QVector<bool> &monoBits,
                                 const QByteArray &monoBuffer,
                                 const QByteArray &grayscale8,
                                 const QVector<quint16> &rgb565,
                                 const QByteArray &rgb888,
                                 const QByteArray &rgb233,
                                 const QVector<quint32> &rgb24,
                                 MonoLayout monoLayout = MonoLayout::RowPacked);

    static QString generate(const DisplayProfile &profile,
                            int width,
                            int height,
                            EncodingMode encodingMode,
                            const QVector<bool> &monoBits,
                            const QByteArray &monoBuffer,
                            const QByteArray &grayscale8,
                            const QVector<quint16> &rgb565,
                            const QByteArray &rgb888,
                            const QByteArray &rgb233,
                            const QVector<quint32> &rgb24,
                            const QString &arrayName = QStringLiteral("image_data"),
                            MonoLayout monoLayout = MonoLayout::RowPacked);

    static QString generate(const DisplayProfile &profile,
                            int width,
                            int height,
                            EncodingMode encodingMode,
                            const QVector<bool> &monoBits,
                            const QByteArray &monoBuffer,
                            const QByteArray &grayscale8,
                            const QVector<quint16> &rgb565,
                            const QByteArray &rgb888,
                            const QByteArray &rgb233,
                            const QVector<quint32> &rgb24,
                            const QString &arrayName,
                            MonoLayout monoLayout,
                            CodeGenOptions options);
};

#endif // DISPLAYCODEGENERATOR_H
