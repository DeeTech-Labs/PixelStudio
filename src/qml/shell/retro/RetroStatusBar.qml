import QtQuick
import QtQuick.Controls

pragma Translator: PixelStudio

Rectangle {
    id: root

    required property var studio
    property string statusText: qsTr("Ready")
    property string specsText: ""

    implicitHeight: studio.statusHeight
    color: studio.surfaceInset
    border.width: 1
    border.color: studio.border

    Label {
        anchors.left: parent.left
        anchors.leftMargin: studio.spacingMd
        anchors.verticalCenter: parent.verticalCenter
        text: statusText
        font.family: studio.fontFamily
        font.pixelSize: studio.fontSizeXs
        color: studio.textMuted
    }

    Row {
        anchors.right: parent.right
        anchors.rightMargin: studio.spacingMd
        anchors.verticalCenter: parent.verticalCenter
        spacing: studio.spacingSm

        Label {
            visible: specsText.length > 0
            text: specsText
            font.family: studio.fontFamilyMono
            font.pixelSize: studio.fontSizeXs
            color: studio.textMuted
        }

        Rectangle {
            width: 8
            height: 8
            radius: 4
            anchors.verticalCenter: parent.verticalCenter
            color: studio.accent
            border.width: 1
            border.color: studio.text
        }
    }
}
