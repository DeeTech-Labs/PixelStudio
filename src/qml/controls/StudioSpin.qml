import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

pragma Translator: Controls


Item {
    id: root
    required property var studio
    property string label: ""
    property alias from: spin.from
    property alias to: spin.to
    property alias stepSize: spin.stepSize
    property alias value: spin.value
    property string toolTipText: ""
    signal valueCommitted(int newValue)

    implicitHeight: studio.controlHeight
    Layout.fillWidth: true
    Layout.preferredHeight: studio.controlHeight

    readonly property int _labelWidth: label.length > 0 ? Math.max(22, label.length * 9) : 0

    HoverHandler { id: hover }
    ToolTip.visible: hover.hovered && toolTipText.length > 0
    ToolTip.text: toolTipText

    RowLayout {
        anchors.fill: parent
        spacing: studio.spacingSm

        Label {
            visible: label.length > 0
            text: label
            Layout.preferredWidth: root._labelWidth
            Layout.maximumWidth: root._labelWidth
            font.family: studio.fontFamily
            font.pixelSize: studio.fontSizeXs
            color: studio.textSecondary
            horizontalAlignment: Text.AlignHCenter
        }

        SpinBox {
            id: spin
            Layout.fillWidth: true
            Layout.minimumWidth: 72
            Layout.preferredHeight: studio.controlHeight
            editable: true
            font.family: studio.fontFamilyMono
            font.pixelSize: studio.fontSizeSm
            onValueModified: root.valueCommitted(value)

            background: Rectangle {
                implicitHeight: studio.controlHeight
                radius: studio.radiusMd
                color: studio.surfaceInput
                border.width: 1
                border.color: spin.activeFocus ? studio.borderFocus : studio.border
            }

            contentItem: TextInput {
                z: 2
                leftPadding: studio.spacingSm
                rightPadding: 28
                text: spin.textFromValue(spin.value, spin.locale)
                font: spin.font
                color: studio.text
                selectionColor: studio.accentMuted
                selectedTextColor: studio.text
                readOnly: !spin.editable
                validator: spin.validator
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            up.indicator: Rectangle {
                x: spin.mirrored ? 2 : spin.width - width - 2
                y: 2
                width: 22
                height: Math.max(12, (spin.height - 4) / 2 - 1)
                radius: studio.radiusSm
                HoverHandler { id: upHover }
                color: upHover.hovered ? studio.surfaceRaised : "transparent"
                Text {
                    anchors.centerIn: parent
                    text: "+"
                    font.pixelSize: studio.fontSizeSm
                    color: studio.textSecondary
                }
            }

            down.indicator: Rectangle {
                x: spin.mirrored ? 2 : spin.width - width - 2
                y: 2 + Math.max(12, (spin.height - 4) / 2 - 1) + 1
                width: 22
                height: Math.max(12, (spin.height - 4) / 2 - 1)
                radius: studio.radiusSm
                HoverHandler { id: downHover }
                color: downHover.hovered ? studio.surfaceRaised : "transparent"
                Text {
                    anchors.centerIn: parent
                    text: "−"
                    font.pixelSize: studio.fontSizeSm
                    color: studio.textSecondary
                }
            }
        }
    }
}
