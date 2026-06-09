import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import PixelStudio

pragma Translator: Shell


Item {
    id: root

    required property var win
    required property var studio
    required property var hostDialogs

    property alias customFolderDialog: customFolderDialog
    property alias settingsExportDialog: settingsExportDialog
    property alias settingsImportDialog: settingsImportDialog
    property alias aboutDialog: aboutDialog
    property alias resetSessionConfirmDialog: resetSessionConfirmDialog

    FolderDialog {
        id: customFolderDialog
        title: qsTr("Choose folder")
        onAccepted: {
            const path = WorkflowRouter.urlToLocalPath(selectedFolder)
            if (win.folderPickTarget === 0)
                workspace.settings.setDocumentsRoot(path)
            else if (win.folderPickTarget === 1)
                workspace.settings.setProjectsRoot(path)
            else
                workspace.settings.setExportsRoot(path)
        }
    }

    FolderDialog {
        id: settingsExportDialog
        title: qsTr("Export settings backup")
        currentFolder: pixelStudioDataPath
        onAccepted: {
            if (workspace.project.exportSettingsTo(selectedFolder))
                WorkflowRouter.statusMessage(qsTr("Settings exported"))
            else
                WorkflowRouter.statusMessage(qsTr("Settings export failed"))
        }
    }

    FolderDialog {
        id: settingsImportDialog
        title: qsTr("Import settings backup")
        currentFolder: pixelStudioDataPath
        onAccepted: {
            if (workspace.project.importSettingsFrom(selectedFolder))
                WorkflowRouter.statusMessage(qsTr("Settings imported"))
            else
                WorkflowRouter.statusMessage(qsTr("Settings import failed"))
        }
    }

    AboutDialog {
        id: aboutDialog
        parent: win.overlay
        anchors.centerIn: parent
        studio: root.studio
        bodyWidth: WorkflowRouter.dialogBodyWidth(440, root.studio.spacingXl * 4)
    }

    Dialog {
        id: resetSessionConfirmDialog
        parent: win.overlay
        anchors.centerIn: parent
        title: qsTr("Reset session?")
        modal: true
        standardButtons: Dialog.NoButton
        padding: root.studio.spacingLg
        property int bodyWidth: WorkflowRouter.dialogBodyWidth(320, root.studio.spacingXl * 4)
        implicitWidth: bodyWidth + leftPadding + rightPadding
        width: implicitWidth
        onAccepted: {
            workspace.project.resetSession()
            WorkflowRouter.statusMessage(qsTr("Session reset"))
        }
        background: Rectangle {
            radius: root.studio.radiusMd
            color: root.studio.surface
            border.width: 2
            border.color: root.studio.accent
        }
        contentItem: ColumnLayout {
            width: resetSessionConfirmDialog.bodyWidth
            spacing: 0
            Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                text: qsTr("Clears inspector settings, recent lists, and watch folders. Projects on disk are not deleted.")
                font.family: root.studio.fontFamily
                color: root.studio.text
            }
        }
        footer: Item {
            width: resetSessionConfirmDialog.bodyWidth
            implicitHeight: resetSessionFooter.implicitHeight
            height: implicitHeight
            StudioConfirmFooter {
                id: resetSessionFooter
                width: parent.width
                studio: root.studio
                dialog: resetSessionConfirmDialog
            }
        }
    }
}
