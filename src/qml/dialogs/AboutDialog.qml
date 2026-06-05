import QtQuick
import QtQuick.Controls
import PixelStudio

pragma Translator: PixelStudio

RetroDialog {
    id: root

    property int bodyWidth: 360

    title: qsTr("About PixelStudio")
    modal: true
    anchors.centerIn: parent
    standardButtons: Dialog.Ok
    width: bodyWidth + 2 * padding

    contentItem: Label {
        width: root.bodyWidth
        wrapMode: Text.WordWrap
        text: qsTr("Converts images to C arrays for OLED and TFT displays.")
        font.family: studio.fontFamily
        color: studio.text
    }
}
