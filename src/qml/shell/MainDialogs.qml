import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import PixelStudio

pragma Translator: Shell


Item {
    id: root
    required property var win
    required property var studio

    ProjectDialogs {
        id: projectDialogs
        win: root.win
        studio: root.studio
    }

    ExportDialogs {
        id: exportDialogs
        win: root.win
    }

    SettingsDialogs {
        id: settingsDialogs
        win: root.win
        studio: root.studio
        hostDialogs: root
    }

    property alias openDialog: projectDialogs.openDialog
    property alias openProjectDialog: projectDialogs.openProjectDialog
    property alias saveProjectDialog: projectDialogs.saveProjectDialog
    property alias importHeaderDialog: projectDialogs.importHeaderDialog
    property alias saveCodeDialog: exportDialogs.saveCodeDialog
    property alias saveBinDialog: exportDialogs.saveBinDialog
    property alias batchOpenDialog: exportDialogs.batchOpenDialog
    property alias atlasOpenDialog: exportDialogs.atlasOpenDialog
    property alias atlasSaveDialog: exportDialogs.atlasSaveDialog
    property alias batchSaveDialog: exportDialogs.batchSaveDialog
    property alias watchInputDialog: exportDialogs.watchInputDialog
    property alias watchOutputDialog: exportDialogs.watchOutputDialog
    property alias customFolderDialog: settingsDialogs.customFolderDialog
    property alias settingsExportDialog: settingsDialogs.settingsExportDialog
    property alias settingsImportDialog: settingsDialogs.settingsImportDialog
    property alias aboutDialog: settingsDialogs.aboutDialog
    property alias resetSessionConfirmDialog: settingsDialogs.resetSessionConfirmDialog
    property alias closeTabConfirmDialog: closeTabConfirmDialog
    property alias exitConfirmDialog: exitConfirmDialog

    Dialog {
        id: closeTabConfirmDialog
        parent: win.overlay
        anchors.centerIn: parent
        title: qsTr("Close tab?")
        modal: true
        standardButtons: Dialog.NoButton
        padding: root.studio.spacingLg
        property int bodyWidth: WorkflowRouter.dialogBodyWidth(360, root.studio.spacingXl * 4)
        width: bodyWidth + 2 * padding
        onAccepted: {
            if (closeTabDontAsk.checked)
                workspace.settings.setConfirmCloseTab(false)
            if (win.pendingCloseTabId.length > 0)
                workspace.tabs.closeTab(win.pendingCloseTabId)
            win.pendingCloseTabId = ""
        }
        onRejected: win.pendingCloseTabId = ""
        background: Rectangle {
            radius: root.studio.radiusMd
            color: root.studio.surface
            border.width: 2
            border.color: root.studio.accent
        }
        contentItem: ColumnLayout {
            width: closeTabConfirmDialog.bodyWidth
            spacing: root.studio.spacingMd
            Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                text: qsTr("Close “%1”? Unsaved changes in this tab may be lost.")
                    .arg(WorkflowRouter.tabTitleForId(win.pendingCloseTabId))
                font.family: root.studio.fontFamily
                color: root.studio.text
            }
            StudioCheck {
                id: closeTabDontAsk
                studio: root.studio
                text: qsTr("Don't ask again")
            }
        }
        footer: StudioConfirmFooter {
            studio: root.studio
            dialog: closeTabConfirmDialog
        }
    }

    ExitConfirmDialog {
        id: exitConfirmDialog
        parent: root.win.overlay
        anchors.centerIn: parent
        studio: root.studio
        win: root.win
        onAccepted: {
            root.win.forceClose = true
            root.win.close()
        }
    }
}
