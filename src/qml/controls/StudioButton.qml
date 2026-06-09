import QtQuick
import QtQuick.Controls

pragma Translator: Controls


Button {
    id: root
    required property var studio
    property bool primary: false
    property bool compact: false
    property string toolTipText: ""
    property string iconName: ""
    property string accessibleName: ""

    implicitHeight: compact ? studio.controlHeightSm : studio.controlHeight

    Accessible.role: Accessible.Button
    Accessible.name: accessibleName.length > 0 ? accessibleName
        : (text.length > 0 ? text : toolTipText)
    leftPadding: iconName.length > 0
        ? (compact ? studio.spacingSm : studio.spacingMd)
        : (compact ? studio.spacingMd : studio.spacingLg)
    rightPadding: compact ? studio.spacingMd : studio.spacingLg

    font.family: studio.fontFamily
    font.pixelSize: compact ? studio.fontSizeSm : studio.fontSizeBase
    font.weight: primary ? Font.DemiBold : Font.Normal

    HoverHandler { id: hover }
    ToolTip.visible: hover.hovered && toolTipText.length > 0
    ToolTip.delay: 400
    ToolTip.text: toolTipText

    background: Rectangle {
        radius: studio.radiusMd
        color: {
            if (!root.enabled) return studio.surfaceInset
            if (root.primary) {
                if (root.pressed) return studio.accentPressed
                if (hover.hovered) return studio.accentHover
                return studio.accent
            }
            if (root.pressed) return studio.surfaceHover
            if (hover.hovered) return studio.surfaceRaised
            return studio.surfaceInput
        }
        border.width: 1
        border.color: root.primary ? studio.accent : studio.border
    }

    topPadding: 0
    bottomPadding: 0

    contentItem: Item {
        implicitWidth: row.implicitWidth
        implicitHeight: row.implicitHeight
        anchors.centerIn: parent

        Row {
            id: row
            anchors.centerIn: parent
            spacing: root.iconName.length > 0 ? studio.spacingSm : 0

            StudioIcon {
                visible: root.iconName.length > 0
                name: root.iconName
                iconSize: root.compact ? 14 : 16
                tint: root.enabled
                    ? (root.primary ? studio.accentOnAccent : studio.text)
                    : studio.textMuted
            }

            Text {
                text: root.text
                font: root.font
                color: root.enabled
                    ? (root.primary ? studio.accentOnAccent : studio.text)
                    : studio.textMuted
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
            }
        }
    }
}
