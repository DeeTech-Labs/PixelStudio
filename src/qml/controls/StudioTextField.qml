import QtQuick
import QtQuick.Controls

pragma Translator: PixelStudio

TextField {
    id: root
    required property var studio
    property string toolTipText: ""

    implicitHeight: studio.controlHeight
    font.family: studio.fontFamily
    font.pixelSize: studio.fontSizeSm
    color: studio.text
    selectionColor: studio.accentMuted
    selectedTextColor: studio.text
    placeholderTextColor: studio.textMuted

    HoverHandler { id: hover }
    ToolTip.visible: hover.hovered && toolTipText.length > 0
    ToolTip.text: toolTipText

    background: Rectangle {
        implicitHeight: studio.controlHeight
        radius: studio.radiusMd
        color: studio.surfaceInput
        border.width: 1
        border.color: root.activeFocus ? studio.borderFocus : studio.border
    }
}
