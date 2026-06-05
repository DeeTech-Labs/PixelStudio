#ifndef PIXELSTUDIO_PROCESSING_PIXELFORMATPACKING_H
#define PIXELSTUDIO_PROCESSING_PIXELFORMATPACKING_H

#include <QByteArray>
#include <QVector>

class PixelFormatPacking
{
public:
    static float srgbChannelToLinear(float channel);
    static quint8 linearChannelToSrgb8(float linear);
    static quint8 quantizeLinearToGray8(float linear);
    static quint16 quantizeLinearToRgb565(float rLin, float gLin, float bLin);
    static quint16 swapRgb565ToBgr565(quint16 rgb565);

    static QByteArray padForDma(QByteArray data, int alignBytes);
    static QByteArray packUint16Le(quint16 value);
    static QByteArray packUint16Be(quint16 value);
    static QByteArray packRgb565Stream(const QVector<quint16> &rgb565, int pixels, bool bigEndian);
    static QByteArray packRgb666Compact(const QByteArray &rgb888, int pixels);
    static QByteArray packHalfLe(const QVector<quint16> &half);
    static QByteArray packFloatLe(const QVector<float> &values);

    static void decodeRgb666Compact(const QByteArray &packed, int pixels, QByteArray &rgb888Out);
    static QByteArray packMonoVerticalCol(const QVector<bool> &bits, int width, int height);

    static void buildIndexedMedianCut(const QByteArray &rgb888,
                                      int pixels,
                                      QByteArray &indicesOut,
                                      QVector<quint32> &paletteRgbOut);

    // YUV: BT.601 full-range (0..255), integer coeffs; chroma = mean of existing 2x2 luma block.
    static QByteArray packYuvNv12(const QByteArray &rgb888, int width, int height);
    static QByteArray packYuvYv12(const QByteArray &rgb888, int width, int height);
    static QByteArray packYuvYuyv(const QByteArray &rgb888, int width, int height);

    static void rgbToYuv(quint8 r, quint8 g, quint8 b, quint8 *y, quint8 *u, quint8 *v);
    static void yuvToRgb(quint8 y, quint8 u, quint8 v, int *r, int *g, int *b);
};

#endif // PIXELSTUDIO_PROCESSING_PIXELFORMATPACKING_H
