pragma Singleton

import QtQuick

pragma Translator: Shell

QtObject {
    function tr(message, context) {
        return qsTranslate(context || "Shell", message)
    }
}
