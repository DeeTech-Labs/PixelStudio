import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import PixelStudio

pragma Translator: PixelStudio

Rectangle {
    id: root
    required property var studio
    implicitHeight: 40
    color: studio.surfaceInset

    signal closeTabRequested(string tabId)
    signal openImageTabRequested()

    Rectangle {
        anchors.bottom: parent.bottom
        width: parent.width
        height: 1
        color: studio.divider
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: studio.spacingMd
        anchors.rightMargin: studio.spacingMd
        spacing: studio.spacingSm

        ScrollView {
            id: tabScroll
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            ScrollBar.vertical.policy: ScrollBar.AlwaysOff

            Row {
                id: tabBarRow
                height: tabScroll.height
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
                    model: tabController.tabs
                    delegate: StudioTabChip {
                        required property var modelData
                        required property int index
                        studio: root.studio
                        tab: modelData
                        tabIndex: index
                        tabBar: tabBarRow
                        tabBarHeight: tabBarRow.height - studio.spacingSm * 2
                        anchors.verticalCenter: parent.verticalCenter
                        onCloseRequested: root.closeTabRequested(tab.id)
                    }
                }
            }
        }

        ToolButton {
            implicitWidth: 32
            implicitHeight: 28
            ToolTip.visible: hovered
            ToolTip.text: qsTr("Open image in new tab…")
            onClicked: root.openImageTabRequested()
            background: Rectangle {
                radius: studio.radiusMd
                color: parent.hovered ? studio.surfaceRaised : studio.surfaceInput
                border.width: 1
                border.color: studio.border
            }
            contentItem: StudioIcon {
                anchors.centerIn: parent
                name: "plus"
                iconSize: 14
                tint: parent.hovered ? studio.accent : studio.textSecondary
            }
        }
    }
}
