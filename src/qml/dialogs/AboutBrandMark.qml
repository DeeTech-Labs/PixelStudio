import QtQuick

pragma Translator: PixelStudio

Item {
    id: root

    required property var studio

    implicitWidth: 72
    implicitHeight: 72

    readonly property color frameColor: studio.accent
    readonly property color screenColor: studio.backgroundElevated

    Rectangle {
        x: 10
        y: 6
        width: 52
        height: 40
        color: "transparent"
        border.width: 2
        border.color: root.frameColor
    }

    Rectangle {
        x: 16
        y: 12
        width: 40
        height: 26
        color: root.screenColor
        border.width: 1
        border.color: Qt.rgba(root.frameColor.r, root.frameColor.g, root.frameColor.b, 0.35)
    }

    Text {
        anchors.centerIn: parent
        anchors.verticalCenterOffset: -6
        text: "P"
        font.family: studio.fontFamilyPixel
        font.pixelSize: 18
        color: studio.text
    }

    Rectangle {
        x: 34
        y: 46
        width: 4
        height: 10
        color: root.frameColor
    }

    Rectangle {
        x: 22
        y: 56
        width: 28
        height: 3
        color: root.frameColor
    }
}
