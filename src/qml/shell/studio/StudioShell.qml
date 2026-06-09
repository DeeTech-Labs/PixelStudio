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

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        StudioTabBar {
            Layout.fillWidth: true
            Layout.preferredHeight: studio.tabBarHeight
            Layout.minimumHeight: studio.tabBarHeight
            studio: root.studio
            onCloseTabRequested: (tabId) => WorkflowRouter.requestCloseTab(tabId)
            onOpenImageTabRequested: WorkflowRouter.run("openImage")
            onSettingsRequested: WorkflowRouter.run("preferences")
        }

        StudioToolbar {
            Layout.fillWidth: true
            Layout.preferredHeight: studio.toolbarHeight
            Layout.minimumHeight: studio.toolbarHeight
            visible: !workspace.tabs.activeIsWelcome && !workspace.tabs.activeIsSettings
            studio: root.studio
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            StudioBackdrop {
                anchors.fill: parent
                visible: workspace.tabs.activeIsWelcome
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
