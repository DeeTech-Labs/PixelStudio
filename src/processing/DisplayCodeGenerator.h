#ifndef DISPLAYCODEGENERATOR_H
#define DISPLAYCODEGENERATOR_H

#include <QString>
#include <QByteArray>
#include <QVector>
#include <QVariantList>

#include "processing/DisplayProfile.h"

struct DisplayCodeGenOptions {
    bool includeHeaderComments = true;
    bool useProgmem = true;
    bool staticStorage = true;
    bool rgb565BigEndian = false;
    int dmaPaddingAlign = 4;
};

class DisplayCodeGenerator
{
public:
    using CodeGenOptions = DisplayCodeGenOptions;

    enum class EncodingMode {
        Mono1Bit = 0,
        Grayscale4 = 1,
        Grayscale8 = 2,
        Indexed8 = 3,
        Rgb565 = 4,
        Rgb666 = 5,
        Rgb888 = 6,
        Argb8888 = 7,
        Bgr565 = 8,
        Bgr888 = 9,
        Abgr8888 = 10,
        YuvNv12 = 11,
        YuvYuyv = 12,
        YuvYv12 = 13,
        R16f = 14,
        Rgba32f = 15,
        Count = 16
    };

    enum class MonoLayout {
        RowPacked = 0,
        Ssd1306Page = 1,
        VerticalColumn = 2
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
                                 MonoLayout monoLayout = MonoLayout::RowPacked,
                                 const CodeGenOptions &options = CodeGenOptions{});

    static int flashFootprintBytes(EncodingMode mode,
                                   int width,
                                   int height,
                                   const QVector<bool> &monoBits,
                                   const QByteArray &monoBuffer,
                                   const QByteArray &grayscale8,
                                   const QVector<quint16> &rgb565,
                                   const QByteArray &rgb888,
                                   const QByteArray &rgb233,
                                   const QVector<quint32> &rgb24,
                                   MonoLayout monoLayout,
                                   const CodeGenOptions &options,
                                   const QVector<quint32> &indexedPalette = {});

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
                            MonoLayout monoLayout = MonoLayout::RowPacked,
                            const QVector<quint32> &indexedPalette = {});

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
                            CodeGenOptions options,
                            const QVector<quint32> &indexedPalette = {});
};

#endif // DISPLAYCODEGENERATOR_H
