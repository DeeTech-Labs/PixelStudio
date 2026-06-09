import QtQuick
import QtQuick.Dialogs
import PixelStudio

pragma Translator: Shell


Item {
    id: root

    required property var win

    property alias saveCodeDialog: saveCodeDialog
    property alias saveBinDialog: saveBinDialog
    property alias batchOpenDialog: batchOpenDialog
    property alias atlasOpenDialog: atlasOpenDialog
    property alias atlasSaveDialog: atlasSaveDialog
    property alias batchSaveDialog: batchSaveDialog
    property alias watchInputDialog: watchInputDialog
    property alias watchOutputDialog: watchOutputDialog

    FileDialog {
        id: saveCodeDialog
        title: qsTr("Save code")
        fileMode: FileDialog.SaveFile
        currentFolder: WorkflowRouter.localFolderUrl(workspace.exportPanel.lastExportDir, workspace.settings.exportsUrl)
        currentFile: workspace.exportPanel.suggestedCodeFileUrl()
        defaultSuffix: "h"
        nameFilters: [
            qsTr("C header") + " (*.h)",
            qsTr("Text file") + " (*.txt)",
            qsTr("All files") + " (*)"
        ]
        onAccepted: {
            if (workspace.exportPanel.saveCodeToFile(selectedFile))
                WorkflowRouter.statusMessage(qsTr("File saved"))
        }
    }

    FileDialog {
        id: saveBinDialog
        title: qsTr("Save binary")
        fileMode: FileDialog.SaveFile
        currentFolder: WorkflowRouter.localFolderUrl(workspace.exportPanel.lastExportDir, workspace.settings.exportsUrl)
        defaultSuffix: "bin"
        nameFilters: [
            qsTr("Binary") + " (*.bin)",
            qsTr("All files") + " (*)"
        ]
        onAccepted: {
            if (workspace.exportPanel.saveBinaryToFile(selectedFile))
                WorkflowRouter.statusMessage(qsTr("File saved"))
        }
    }

    FileDialog {
        id: batchOpenDialog
        title: qsTr("Select images for batch")
        fileMode: FileDialog.OpenFiles
        currentFolder: WorkflowRouter.localFolderUrl(workspace.project.lastOpenImageDir, workspace.settings.projectsUrl)
        nameFilters: [
            qsTr("Images") + " (*.png *.jpg *.jpeg *.bmp *.gif *.webp)",
            qsTr("All files") + " (*)"
        ]
        onAccepted: {
            win.pendingBatchFiles = selectedFiles
            if (win.pendingBatchFiles.length > 0)
                batchSaveDialog.open()
        }
    }

    FileDialog {
        id: atlasOpenDialog
        title: qsTr("Select images for atlas")
        fileMode: FileDialog.OpenFiles
        currentFolder: WorkflowRouter.localFolderUrl(workspace.project.lastOpenImageDir, workspace.settings.projectsUrl)
        nameFilters: [
            qsTr("Images") + " (*.png *.jpg *.jpeg *.bmp *.gif *.webp)",
            qsTr("All files") + " (*)"
        ]
        onAccepted: {
            win.pendingAtlasFiles = selectedFiles
            if (win.pendingAtlasFiles.length > 0)
                atlasSaveDialog.open()
        }
    }

    FileDialog {
        id: atlasSaveDialog
        title: qsTr("Save atlas header")
        fileMode: FileDialog.SaveFile
        currentFolder: WorkflowRouter.localFolderUrl(workspace.exportPanel.lastExportDir, workspace.settings.exportsUrl)
        defaultSuffix: "h"
        nameFilters: [ qsTr("C header") + " (*.h)" ]
        onAccepted: {
            if (workspace.exportPanel.buildSpriteAtlas(win.pendingAtlasFiles, selectedFile,
                                                        workspace.output.displayWidth, workspace.output.displayHeight))
                WorkflowRouter.statusMessage(qsTr("Atlas saved"))
        }
    }

    FileDialog {
        id: batchSaveDialog
        title: qsTr("Save batch header")
        fileMode: FileDialog.SaveFile
        currentFolder: WorkflowRouter.localFolderUrl(workspace.exportPanel.lastExportDir, workspace.settings.exportsUrl)
        defaultSuffix: "h"
        nameFilters: [ qsTr("C header") + " (*.h)" ]
        onAccepted: {
            workspace.exportPanel.enqueueBatchCodeExport(win.pendingBatchFiles, selectedFile)
            WorkflowRouter.statusMessage(qsTr("Batch queued"))
        }
    }

    FolderDialog {
        id: watchInputDialog
        title: qsTr("Select watch input folder")
        onAccepted: watchOutputDialog.open()
    }

    FolderDialog {
        id: watchOutputDialog
        title: qsTr("Select watch output folder")
        currentFolder: workspace.settings.watchUrl
        onAccepted: {
            workspace.exportPanel.configureWatchFolders(watchInputDialog.selectedFolder, selectedFolder)
            workspace.exportPanel.setWatchFolderActive(true)
            WorkflowRouter.statusMessage(qsTr("Watch folder enabled"))
        }
    }
}
