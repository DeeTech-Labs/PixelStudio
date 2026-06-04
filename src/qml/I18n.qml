pragma Singleton

import QtQuick

pragma Translator: PixelStudio

QtObject {
    function tr(message) {
        return qsTranslate("PixelStudio", message)
    }
}
