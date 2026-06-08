import QtQuick
import QtQuick.Controls

pragma Translator: Workspace


Item {
    id: root
    default property alias body: bodySlot.data

    required property var studio
    property string title: ""
    property bool showBrackets: false

    implicitWidth: 200
    implicitHeight: header.height + bodySlot.childrenRect.height + studio.spacingSm * 2

    readonly property int bracket: studio.panelBracketSize

    Rectangle {
        anchors.fill: parent
        radius: studio.radiusMd
        color: studio.surfaceInset
        border.width: studio.pixelBorderWidth
        border.color: studio.border
        clip: true
    }

    Rectangle {
        visible: showBrackets
        width: bracket
        height: bracket
        color: studio.accent
        opacity: 0.65
    }
    Rectangle {
        visible: showBrackets
        anchors.right: parent.right
        width: bracket
        height: bracket
        color: studio.accent
        opacity: 0.65
    }
    Rectangle {
        visible: showBrackets
        anchors.bottom: parent.bottom
        width: bracket
        height: bracket
        color: studio.accent
        opacity: 0.65
    }
    Rectangle {
        visible: showBrackets
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        width: bracket
        height: bracket
        color: studio.accent
        opacity: 0.65
    }

    Rectangle {
        id: header
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 1
        height: title.length > 0 ? 22 : 0
        visible: title.length > 0
        color: studio.surface

        Label {
            anchors.left: parent.left
            anchors.leftMargin: studio.spacingSm
            anchors.verticalCenter: parent.verticalCenter
            text: title.toUpperCase()
            font.family: studio.fontFamilyPixel
            font.pixelSize: studio.fontSizePixel
            color: studio.accent
        }

        Rectangle {
            anchors.bottom: parent.bottom
            width: parent.width
            height: 1
            color: studio.border
        }
    }

    Item {
        id: bodySlot
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: header.bottom
        anchors.bottom: parent.bottom
        anchors.margins: studio.spacingSm
        clip: true
    }
}
