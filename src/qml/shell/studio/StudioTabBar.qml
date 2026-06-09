import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import PixelStudio

pragma Translator: Shell


Rectangle {
    id: root

    required property var studio

    height: studio.tabBarHeight
    implicitHeight: studio.tabBarHeight

    signal closeTabRequested(string tabId)
    signal openImageTabRequested()
    signal settingsRequested()

    color: studio.surfaceInset
    border.width: 1
    border.color: studio.border

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: studio.spacingSm
        anchors.rightMargin: studio.spacingSm
        spacing: studio.spacingXs

        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: studio.tabBarHeight - 4
            clip: true

            Flickable {
                id: tabFlick
                anchors.fill: parent
                contentWidth: tabBarRow.width
                contentHeight: height
                boundsBehavior: Flickable.StopAtBounds
                flickableDirection: Flickable.HorizontalFlick

                Row {
                    id: tabBarRow
                    height: tabFlick.height
                    spacing: studio.spacingXs

                    function dropIndexAt(localX) {
                        for (let i = 0; i < tabRepeater.count; ++i) {
                            const item = tabRepeater.itemAt(i)
                            if (!item)
                                continue
                            const mid = item.x + item.width * 0.5
                            if (localX < mid)
                                return i
                        }
                        return Math.max(0, tabRepeater.count - 1)
                    }

                    Repeater {
                        id: tabRepeater
                        model: workspace.tabs.tabs
                        delegate: StudioTabChip {
                            required property var modelData
                            required property int index
                            studio: root.studio
                            tab: modelData
                            tabIndex: index
                            tabBar: tabBarRow
                            tabBarHeight: tabBarRow.height
                            anchors.verticalCenter: parent.verticalCenter
                            onCloseRequested: root.closeTabRequested(tab.id)
                        }
                    }
                }
            }
        }

        ToolButton {
            implicitWidth: 28
            implicitHeight: 26
            ToolTip.visible: hovered
            ToolTip.text: WorkflowRouter.shellTr("Open image in new tab…")
            onClicked: root.openImageTabRequested()
            background: Rectangle {
                color: parent.hovered ? studio.surfaceRaised : studio.surfaceInput
                border.width: 1
                border.color: studio.border
            }
            contentItem: Label {
                text: "+"
                font.family: studio.fontFamilyPixel
                font.pixelSize: studio.fontSizePixel
                color: parent.hovered ? studio.accent : studio.textSecondary
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }

        ToolButton {
            implicitWidth: 28
            implicitHeight: 26
            ToolTip.visible: hovered
            ToolTip.text: WorkflowRouter.shellTr("Settings")
            onClicked: root.settingsRequested()
            background: Rectangle {
                color: parent.hovered ? studio.surfaceRaised : "transparent"
                border.width: 1
                border.color: studio.border
            }
            contentItem: StudioIcon {
                anchors.centerIn: parent
                name: "settings"
                iconSize: 14
                tint: parent.hovered ? studio.accent : studio.textMuted
            }
        }
    }
}
