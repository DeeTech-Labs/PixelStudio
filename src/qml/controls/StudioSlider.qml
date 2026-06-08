import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

pragma Translator: PixelStudio

Item {
    id: root
    required property var studio
    property string label: ""
    property alias from: slider.from
    property alias to: slider.to
    property alias stepSize: slider.stepSize
    property alias value: slider.value
    readonly property alias pressed: slider.pressed
    property string valueText: ""
    property bool liveUpdate: false
    property string toolTipText: ""

    signal valueCommitted(real newValue)

    implicitHeight: col.implicitHeight

    HoverHandler { id: hover }
    ToolTip.visible: hover.hovered && toolTipText.length > 0
    ToolTip.text: toolTipText

    ColumnLayout {
        id: col
        width: parent.width
        spacing: studio.spacingXs

        RowLayout {
            Layout.fillWidth: true
            Label {
                text: label
                font.family: studio.fontFamily
                font.pixelSize: studio.fontSizeXs
                color: studio.textSecondary
            }
            Item { Layout.fillWidth: true }
            Label {
                Layout.maximumWidth: 72
                horizontalAlignment: Text.AlignRight
                elide: Text.ElideLeft
                text: valueText.length ? valueText : Math.round(slider.value)
                font.family: studio.fontFamilyMono
                font.pixelSize: studio.fontSizeXs
                color: studio.text
            }
        }

        Slider {
            id: slider
            Layout.fillWidth: true
            Layout.preferredHeight: 22
            onMoved: if (liveUpdate) root.valueCommitted(value)
            onPressedChanged: if (!pressed) root.valueCommitted(value)

            background: Item {
                x: slider.leftPadding
                y: (slider.height - height) / 2
                width: slider.availableWidth
                height: 6
                Rectangle {
                    width: parent.width
                    height: parent.height
                    radius: studio.radiusPill
                    color: studio.surfaceInset
                    border.width: 1
                    border.color: studio.border
                }
                Rectangle {
                    width: slider.visualPosition * parent.width
                    height: parent.height
                    radius: studio.radiusPill
                    color: studio.accent
                }
            }

            handle: Rectangle {
                x: slider.leftPadding + slider.visualPosition * (slider.availableWidth - width)
                y: (slider.height - height) / 2
                width: 12
                height: 12
                radius: studio.radiusSm
                color: slider.pressed ? studio.accentPressed
                    : (slider.hovered ? studio.accentHover : studio.accent)
                border.width: 1
                border.color: studio.text
            }
        }
    }
}
