import QtQuick
import QtQuick.Controls

pragma Translator: PixelStudio

Button {
    id: root
    required property var studio
    property string iconName: ""
    property string iconText: ""
    property bool small: false
    property bool toggled: false
    property string toolTipText: ""

    implicitWidth: small ? 26 : 32
    implicitHeight: implicitWidth
    text: iconText.length ? iconText : root.text

    font.family: studio.fontFamily
    font.pixelSize: small ? studio.fontSizeSm : studio.fontSizeBase
    font.weight: Font.Medium

    HoverHandler { id: hover }
    ToolTip.visible: hover.hovered && toolTipText.length > 0
    ToolTip.text: toolTipText

    background: Rectangle {
        radius: studio.radiusSm
        color: root.toggled ? studio.accentSoft
            : (root.pressed ? studio.surfaceHover
               : (hover.hovered ? studio.railHover : "transparent"))
        border.width: root.toggled ? 1 : 0
        border.color: root.toggled ? studio.accentMuted : "transparent"
    }

    contentItem: Item {
        implicitWidth: icon.implicitWidth
        implicitHeight: icon.implicitHeight
        anchors.centerIn: parent

        StudioIcon {
            id: icon
            visible: root.iconName.length > 0
            anchors.centerIn: parent
            name: root.iconName
            iconSize: root.small ? 14 : 16
            tint: root.enabled
                ? (root.toggled ? studio.accent : studio.text)
                : studio.textMuted
        }

        Text {
            visible: root.iconName.length === 0
            anchors.centerIn: parent
            text: root.text
            font: root.font
            color: root.enabled ? studio.text : studio.textMuted
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }
}
