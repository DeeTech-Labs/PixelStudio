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
        implicitWidth: 36
        implicitHeight: 16
        radius: studio.radiusPill
        color: studio.surfaceInset
        border.width: 1
        border.color: studio.border
        Rectangle {
            width: 16
            height: 12
            radius: studio.radiusSm
            anchors.verticalCenter: parent.verticalCenter
            x: root.checked ? parent.width - width - 2 : 2
            color: root.checked ? studio.accent : studio.decorativeDisabled
            border.width: 1
            border.color: root.checked ? studio.accent : studio.border
            Behavior on x { NumberAnimation { duration: studio.durationFast } }
        }
    }

    contentItem: Text {
        width: root.width > 0 ? root.width : implicitWidth
        leftPadding: root.indicator.width + root.spacing
        text: root.text
        font: root.font
        color: studio.text
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
}
