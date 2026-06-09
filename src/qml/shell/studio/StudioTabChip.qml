import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import PixelStudio

pragma Translator: Shell


Rectangle {
    id: chip

    required property var studio
    required property var tab
    required property int tabIndex
    required property var tabBar

    signal closeRequested()

    property bool isActive: tab.id === workspace.tabs.activeTabId
    property bool dragging: false
    property int pendingDropIndex: -1
    property int tabBarHeight: 28
    property string contextTabId: tab.id

    implicitWidth: Math.max(96, tabRow.implicitWidth + studio.spacingMd * 2)
    implicitHeight: tabBarHeight
    radius: studio.radiusSm
    color: isActive ? studio.surfaceRaised : studio.surface
    border.width: studio.pixelBorderWidth
    border.color: isActive ? studio.accent : studio.border
    opacity: dragging ? 0.78 : 1
    z: dragging ? 20 : 0

    Rectangle {
        visible: isActive
        anchors.bottom: parent.bottom
        width: parent.width
        height: 2
        color: studio.accent
    }

    Menu {
        id: tabContextMenu
        MenuItem {
            text: WorkflowRouter.shellTr("Close tab")
            enabled: tab.closable
            onTriggered: chip.closeRequested()
        }
        MenuItem {
            text: WorkflowRouter.shellTr("Close other tabs")
            enabled: tab.closable
            onTriggered: workspace.tabs.closeOtherTabs(contextTabId)
        }
        MenuSeparator {}
        MenuItem {
            text: tab.pinned ? WorkflowRouter.shellTr("Unpin tab") : WorkflowRouter.shellTr("Pin tab")
            enabled: !tab.isWelcome && !tab.isSettings
            onTriggered: workspace.tabs.setTabPinned(contextTabId, !tab.pinned)
        }
        MenuSeparator {}
        MenuItem {
            text: WorkflowRouter.shellTr("Move left")
            enabled: tabIndex > (workspace.tabs.tabs.length > 0 && workspace.tabs.tabs[0].isWelcome ? 1 : 0)
            onTriggered: workspace.tabs.moveTab(contextTabId, tabIndex - 1)
        }
        MenuItem {
            text: WorkflowRouter.shellTr("Move right")
            enabled: tabIndex < workspace.tabs.tabs.length - 1
            onTriggered: workspace.tabs.moveTab(contextTabId, tabIndex + 1)
        }
    }

    RowLayout {
        id: tabRow
        anchors.centerIn: parent
        spacing: studio.spacingXs

        StudioIcon {
            visible: tab.isWelcome
            name: "house"
            iconSize: 11
            tint: isActive ? studio.accent : studio.textMuted
        }
        StudioIcon {
            visible: tab.isSettings
            name: "settings"
            iconSize: 11
            tint: isActive ? studio.accent : studio.textMuted
        }

        Label {
            Layout.maximumWidth: 140
            text: tab.title
            font.family: isActive ? studio.fontFamilyPixel : studio.fontFamily
            font.pixelSize: isActive ? studio.fontSizePixel : studio.fontSizeSm
            color: isActive ? studio.accent : studio.textSecondary
            elide: Text.ElideRight
        }

        ToolButton {
            id: closeBtn
            visible: tab.closable
            implicitWidth: 18
            implicitHeight: 18
            onClicked: chip.closeRequested()
            background: Rectangle {
                radius: studio.radiusSm
                color: closeBtn.hovered ? studio.error : "transparent"
                opacity: closeBtn.hovered ? 0.35 : 0
            }
            contentItem: Label {
                text: "×"
                font.pixelSize: studio.fontSizeSm
                color: studio.textSecondary
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }
    }

    MouseArea {
        id: tabMouse
        anchors.fill: parent
        anchors.rightMargin: tab.closable ? 22 : 0
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        cursorShape: Qt.PointingHandCursor

        property real pressX: 0
        property bool didDrag: false

        onPressed: (mouse) => {
            pressX = mouse.x
            didDrag = false
            chip.dragging = false
            chip.pendingDropIndex = -1
            if (mouse.button === Qt.RightButton) {
                contextTabId = tab.id
                tabContextMenu.popup(chip)
                mouse.accepted = true
            }
        }
        onPositionChanged: (mouse) => {
            if (tab.isWelcome || tab.isSettings || !(mouse.buttons & Qt.LeftButton))
                return
            if (!chip.dragging && Math.abs(mouse.x - pressX) > 8)
                chip.dragging = true
            if (!chip.dragging)
                return
            didDrag = true
            const pos = mapToItem(tabBar, mouse.x, mouse.y)
            chip.pendingDropIndex = tabBar.dropIndexAt(pos.x)
        }
        onReleased: {
            if (chip.dragging && chip.pendingDropIndex >= 0 && chip.pendingDropIndex !== tabIndex)
                workspace.tabs.moveTab(tab.id, chip.pendingDropIndex)
            chip.dragging = false
            chip.pendingDropIndex = -1
        }
        onClicked: (mouse) => {
            if (mouse.button === Qt.LeftButton && !didDrag)
                workspace.tabs.activateTab(tab.id)
        }
    }
}
