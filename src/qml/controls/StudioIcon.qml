import QtQuick
import Qt5Compat.GraphicalEffects

pragma Translator: Controls


Item {
    id: root
    required property string name
    property int iconSize: 16
    property color tint: "#C8D4E0"

    implicitWidth: iconSize
    implicitHeight: iconSize

    Image {
        id: glyph
        anchors.fill: parent
        source: name.length > 0 ? ("qrc:/icons/" + name + ".png") : ""
        sourceSize: Qt.size(root.iconSize * 2, root.iconSize * 2)
        fillMode: Image.PreserveAspectFit
        smooth: false
        visible: false
    }

    ColorOverlay {
        anchors.fill: parent
        source: glyph
        color: root.tint
    }
}
