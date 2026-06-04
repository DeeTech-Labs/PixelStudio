#ifndef DISPLAYPROFILE_H
#define DISPLAYPROFILE_H

#include <QString>
#include <QVector>

struct DisplayProfile
{
    enum ColorMode {
        Mono1Bit = 0,
        Rgb565   = 1
    };

    enum ScaleMode {
        Fit     = 0,
        Stretch = 1,
        Crop    = 2
    };

    QString id;
    QString name;
    int width = 128;
    int height = 64;
    ColorMode colorMode = Mono1Bit;

    static QVector<DisplayProfile> presets();
    static DisplayProfile byId(const QString &id);
};

#endif // DISPLAYPROFILE_H
