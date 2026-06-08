import QtQuick
import QtQuick.Controls

pragma Translator: Shell


Rectangle {
    id: root

    required property var studio
    property string text: ""

    visible: text.length > 0
    implicitWidth: label.implicitWidth + studio.spacingXl * 2
    implicitHeight: 32
    color: studio.accent
    border.width: 2
    border.color: studio.text

    Label {
        id: label
        anchors.centerIn: parent
        text: root.text
        font.family: studio.fontFamilyPixel
        font.pixelSize: studio.fontSizePixel
        color: studio.accentOnAccent
    }
}
