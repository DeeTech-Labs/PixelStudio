import QtQuick
import QtQuick.Controls

pragma Translator: Controls


Button {
    id: root
    required property var studio
    property string iconName: ""
    property string iconText: ""
    property bool small: false
    property bool toggled: false
    property string toolTipText: ""
    property string accessibleName: ""

    readonly property int _textPadH: small ? studio.spacingSm : studio.spacingMd

    Accessible.role: Accessible.Button
    Accessible.name: accessibleName.length > 0 ? accessibleName
        : (toolTipText.length > 0 ? toolTipText
           : (text.length > 0 ? text : iconText))
    readonly property int _minSide: small ? 26 : 32

    implicitWidth: iconName.length > 0
        ? _minSide
        : Math.max(_minSide, labelText.implicitWidth + _textPadH * 2)
    implicitHeight: _minSide
    leftPadding: iconName.length > 0 ? 0 : _textPadH
    rightPadding: leftPadding
    text: iconText.length ? iconText : root.text

    font.family: studio.fontFamily
    font.pixelSize: small ? studio.fontSizeSm : studio.fontSizeBase
    font.weight: Font.Medium

    HoverHandler { id: hover }
    ToolTip.visible: hover.hovered && toolTipText.length > 0
    ToolTip.text: toolTipText

    background: Rectangle {
        radius: studio.radiusMd
        color: root.toggled ? studio.accentSoft
            : (root.pressed ? studio.surfaceHover
               : (hover.hovered ? studio.railHover : "transparent"))
        border.width: root.toggled ? 1 : 0
        border.color: root.toggled ? studio.accentMuted : "transparent"
    }

    contentItem: Item {
        implicitWidth: root.iconName.length > 0 ? icon.implicitWidth : labelText.implicitWidth
        implicitHeight: root.iconName.length > 0 ? icon.implicitHeight : labelText.implicitHeight
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
            id: labelText
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
