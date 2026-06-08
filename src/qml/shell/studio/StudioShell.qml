import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import PixelStudio

pragma Translator: Shell


Item {
    id: root

    required property var studio
    default property alias content: contentSlot.data

    property string statusText: qsTr("Ready")
    property string specsText: ""
    property string toastText: ""

    signal closeTabRequested(string tabId)
    signal openImageTabRequested()
    signal settingsRequested()
    signal newProjectRequested()
    signal openRequested()
    signal saveRequested()
    signal toggleGridRequested()
    signal viewDualRequested()
    signal viewSourceRequested()
    signal viewOutputRequested()

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        StudioTabBar {
            Layout.fillWidth: true
            Layout.preferredHeight: studio.tabBarHeight
            Layout.minimumHeight: studio.tabBarHeight
            studio: root.studio
            onCloseTabRequested: (tabId) => root.closeTabRequested(tabId)
            onOpenImageTabRequested: root.openImageTabRequested()
            onSettingsRequested: root.settingsRequested()
        }

        StudioToolbar {
            Layout.fillWidth: true
            Layout.preferredHeight: studio.toolbarHeight
            Layout.minimumHeight: studio.toolbarHeight
            visible: !tabController.activeIsWelcome
            studio: root.studio
            onNewProjectRequested: root.newProjectRequested()
            onOpenRequested: root.openRequested()
            onSaveRequested: root.saveRequested()
            onToggleGridRequested: root.toggleGridRequested()
            onViewDualRequested: root.viewDualRequested()
            onViewSourceRequested: root.viewSourceRequested()
            onViewOutputRequested: root.viewOutputRequested()
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            StudioBackdrop {
                anchors.fill: parent
                visible: tabController.activeIsWelcome
            }

            Item {
                id: contentSlot
                anchors.fill: parent
            }
        }

        StudioStatusBar {
            Layout.fillWidth: true
            studio: root.studio
            statusText: root.statusText
            specsText: root.specsText
        }
    }

    StudioToast {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: studio.statusHeight + studio.spacingMd
        studio: root.studio
        text: root.toastText
        z: 100
    }
}
