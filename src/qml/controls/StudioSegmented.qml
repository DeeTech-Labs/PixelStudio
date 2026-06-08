import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

pragma Translator: Controls


Item {
    id: root
    required property var studio
    property var segments: []
    property int selectedValue: 0
    signal segmentActivated(int value)

    implicitHeight: studio.controlHeight
    implicitWidth: row.implicitWidth
    width: parent ? parent.width : implicitWidth

    Rectangle {
        anchors.fill: parent
        radius: studio.radiusMd
        color: studio.surfaceInset
        border.width: 1
        border.color: studio.border
    }

    RowLayout {
        id: row
        anchors.fill: parent
        anchors.margins: 2
        spacing: 2

        Repeater {
            model: root.segments
            delegate: Button {
                required property var modelData
                required property int index
                Layout.fillWidth: true
                Layout.preferredHeight: root.height - 4
                text: modelData.label
                font.family: studio.fontFamily
                font.pixelSize: studio.fontSizeSm
                flat: true
                topPadding: 0
                bottomPadding: 0
                leftPadding: studio.spacingSm
                rightPadding: studio.spacingSm
                background: Rectangle {
                    radius: studio.radiusSm
                    color: modelData.value === root.selectedValue
                        ? studio.surfaceRaised : "transparent"
                    border.width: modelData.value === root.selectedValue ? 1 : 0
                    border.color: studio.border
                }
                contentItem: Text {
                    width: parent.width
                    text: parent.text
                    font: parent.font
                    color: modelData.value === root.selectedValue ? studio.text : studio.textSecondary
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }
                onClicked: {
                    root.selectedValue = modelData.value
                    root.segmentActivated(modelData.value)
                }
            }
        }
    }
}
