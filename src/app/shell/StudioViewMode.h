#ifndef PIXELSTUDIO_APP_SHELL_STUDIOVIEWMODE_H
#define PIXELSTUDIO_APP_SHELL_STUDIOVIEWMODE_H

#include <QObject>

namespace StudioViewMode {

Q_NAMESPACE

enum Mode {
    Dual = 0,
    Source = 1,
    Output = 3
};
Q_ENUM_NS(Mode)

} // namespace StudioViewMode

#endif // PIXELSTUDIO_APP_SHELL_STUDIOVIEWMODE_H
