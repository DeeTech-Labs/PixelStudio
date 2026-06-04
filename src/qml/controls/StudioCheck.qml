import QtQuick
import QtQuick.Controls

pragma Translator: PixelStudio

CheckBox {
    id: root
    required property var studio
    property string toolTipText: ""

    implicitHeight: studio.controlHeightSm
    font.family: studio.fontFamily
    font.pixelSize: studio.fontSizeSm
    spacing: studio.spacingSm

    HoverHandler { id: hover }
    ToolTip.visible: hover.hovered && toolTipText.length > 0
    ToolTip.text: toolTipText

    indicator: Rectangle {
        implicitWidth: 18
        implicitHeight: 18
        radius: studio.radiusSm
        color: root.checked ? studio.accent : studio.surfaceInput
        border.width: root.checked ? 0 : 1
        border.color: root.checked ? studio.accent : studio.textMuted
        Text {
            anchors.centerIn: parent
            visible: root.checked
            text: "✓"
            font.pixelSize: 11
            color: studio.accentOnAccent
        }
    }

    contentItem: Text {
        leftPadding: root.indicator.width + root.spacing
        text: root.text
        font: root.font
        color: studio.text
        verticalAlignment: Text.AlignVCenter
    }
}
