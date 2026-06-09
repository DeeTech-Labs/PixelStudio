import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import PixelStudio

pragma Translator: Shell


Rectangle {
    id: root

    required property var studio

    implicitHeight: studio.toolbarHeight
    color: studio.surface
    border.width: 1
    border.color: studio.border

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: studio.spacingSm
        anchors.rightMargin: studio.spacingSm
        spacing: studio.spacingXs

        Repeater {
            model: StudioCommands.toolbarItems
            delegate: Item {
                required property var modelData
                Layout.preferredWidth: modelData.sep ? 8 : 28
                Layout.preferredHeight: 28

                Rectangle {
                    visible: modelData.sep === true
                    anchors.centerIn: parent
                    width: 1
                    height: 20
                    color: studio.border
                }

                readonly property bool gridActive: modelData.command === "toggleGrid"
                    && workspace.viewport.showGrid

                ToolButton {
                    visible: modelData.sep !== true
                    anchors.fill: parent
                    enabled: modelData.enabled === true
                    ToolTip.visible: hovered && modelData.tip && modelData.tip.length > 0
                    ToolTip.text: modelData.tip ? WorkflowRouter.shellTr(modelData.tip) : ""
                    onClicked: WorkflowRouter.run(modelData.command)
                    background: Rectangle {
                        radius: studio.radiusSm
                        color: gridActive
                            ? studio.accentSoft
                            : (parent.hovered && parent.enabled ? studio.railHover : "transparent")
                        border.width: gridActive || (parent.enabled && parent.hovered) ? 1 : 0
                        border.color: gridActive ? studio.accent : studio.accent
                    }
                    contentItem: StudioIcon {
                        anchors.centerIn: parent
                        name: modelData.icon || ""
                        iconSize: 16
                        tint: parent.enabled
                            ? (gridActive ? studio.accent : studio.text)
                            : studio.textMuted
                    }
                }
            }
        }

        Item { Layout.fillWidth: true }
    }
}
