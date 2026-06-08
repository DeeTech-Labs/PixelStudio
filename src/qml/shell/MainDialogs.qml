import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import PixelStudio

pragma Translator: PixelStudio

Item {
    id: root
    required property var win
    required property var studio

    property alias openDialog: openDialog
    property alias openProjectDialog: openProjectDialog
    property alias saveProjectDialog: saveProjectDialog
    property alias importHeaderDialog: importHeaderDialog
    property alias saveCodeDialog: saveCodeDialog
    property alias saveBinDialog: saveBinDialog
    property alias batchOpenDialog: batchOpenDialog
    property alias atlasOpenDialog: atlasOpenDialog
    property alias atlasSaveDialog: atlasSaveDialog
    property alias customFolderDialog: customFolderDialog
    property alias watchInputDialog: watchInputDialog
    property alias watchOutputDialog: watchOutputDialog
    property alias settingsExportDialog: settingsExportDialog
    property alias settingsImportDialog: settingsImportDialog
    property alias batchSaveDialog: batchSaveDialog
    property alias aboutDialog: aboutDialog
    property alias syntaxColorsDialog: syntaxColorsDialog
    property alias preferencesDialog: preferencesDialog
    property alias resetSessionConfirmDialog: resetSessionConfirmDialog
    property alias closeTabConfirmDialog: closeTabConfirmDialog
    property alias exitConfirmDialog: exitConfirmDialog

    PreferencesDialog {
        id: preferencesDialog
        parent: win.overlay
        anchors.centerIn: parent
        win: root.win
        theme: root.studio
        hostDialogs: root
    }

    FileDialog {
        id: openDialog
        title: qsTr("Open image")
        currentFolder: win.localFolderUrl(converter.lastOpenImageDir, pixelStudioProjectsUrl)
        nameFilters: [
            qsTr("Images") + " (*.png *.jpg *.jpeg *.bmp *.gif *.webp)",
            qsTr("All files") + " (*)"
        ]
        onAccepted: win.openLocalPath(selectedFile)
    }

    FileDialog {
        id: openProjectDialog
        title: qsTr("Open PixelStudio project")
        currentFolder: pixelStudioProjectsUrl
        nameFilters: [ qsTr("PixelStudio project") + " (*.pspx)", qsTr("All files") + " (*)" ]
        onAccepted: tabController.openProjectTab(selectedFile)
    }

    FileDialog {
        id: saveProjectDialog
        title: qsTr("Save PixelStudio project")
        fileMode: FileDialog.SaveFile
        currentFolder: pixelStudioProjectsUrl
        defaultSuffix: "pspx"
        nameFilters: [ qsTr("PixelStudio project") + " (*.pspx)" ]
        onAccepted: {
            if (converter.saveProjectAs(selectedFile)) {
                tabController.syncActiveTabTitle()
                win.statusMessage(qsTr("Project saved"))
            }
        }
    }

    FileDialog {
        id: importHeaderDialog
        title: qsTr("Import C header")
        currentFolder: win.localFolderUrl(converter.lastOpenImageDir, pixelStudioExportsUrl)
        nameFilters: [ qsTr("C header") + " (*.h *.hpp)", qsTr("All files") + " (*)" ]
        onAccepted: {
            if (tabController.activeIsWelcome)
                tabController.newProjectTab(qsTr("Untitled"))
            if (converter.importHeader(selectedFile))
                tabController.syncActiveTabTitle()
        }
    }

    FileDialog {
        id: saveCodeDialog
        title: qsTr("Save code")
        fileMode: FileDialog.SaveFile
        currentFolder: win.localFolderUrl(converter.lastExportDir, pixelStudioExportsUrl)
        currentFile: converter.suggestedCodeFileUrl()
        defaultSuffix: "h"
        nameFilters: [
            qsTr("C header") + " (*.h)",
            qsTr("Text file") + " (*.txt)",
            qsTr("All files") + " (*)"
        ]
        onAccepted: {
            if (converter.saveCodeToFile(selectedFile))
                win.statusMessage(qsTr("File saved"))
        }
    }

    FileDialog {
        id: saveBinDialog
        title: qsTr("Save binary")
        fileMode: FileDialog.SaveFile
        currentFolder: win.localFolderUrl(converter.lastExportDir, pixelStudioExportsUrl)
        defaultSuffix: "bin"
        nameFilters: [
            qsTr("Binary") + " (*.bin)",
            qsTr("All files") + " (*)"
        ]
        onAccepted: {
            if (converter.saveBinaryToFile(selectedFile))
                win.statusMessage(qsTr("File saved"))
        }
    }

    FileDialog {
        id: batchOpenDialog
        title: qsTr("Select images for batch")
        fileMode: FileDialog.OpenFiles
        currentFolder: win.localFolderUrl(converter.lastOpenImageDir, pixelStudioProjectsUrl)
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
        currentFolder: win.localFolderUrl(converter.lastOpenImageDir, pixelStudioProjectsUrl)
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
        currentFolder: win.localFolderUrl(converter.lastExportDir, pixelStudioExportsUrl)
        defaultSuffix: "h"
        nameFilters: [ qsTr("C header") + " (*.h)" ]
        onAccepted: {
            if (converter.buildSpriteAtlas(win.pendingAtlasFiles, selectedFile, converter.displayWidth, converter.displayHeight))
                win.statusMessage(qsTr("Atlas saved"))
        }
    }

    FolderDialog {
        id: customFolderDialog
        title: qsTr("Choose folder")
        onAccepted: {
            const path = selectedFolder.toLocalFile()
            if (win.folderPickTarget === 0)
                appSettings.setDocumentsRoot(path)
            else if (win.folderPickTarget === 1)
                appSettings.setProjectsRoot(path)
            else
                appSettings.setExportsRoot(path)
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
        currentFolder: pixelStudioWatchUrl
        onAccepted: {
            converter.configureWatchFolders(watchInputDialog.selectedFolder, selectedFolder)
            converter.setWatchFolderActive(true)
            win.statusMessage(qsTr("Watch folder enabled"))
        }
    }

    FolderDialog {
        id: settingsExportDialog
        title: qsTr("Export settings backup")
        currentFolder: pixelStudioDataPath
        onAccepted: {
            if (converter.exportSettingsTo(selectedFolder))
                win.statusMessage(qsTr("Settings exported"))
            else
                win.statusMessage(qsTr("Settings export failed"))
        }
    }

    FolderDialog {
        id: settingsImportDialog
        title: qsTr("Import settings backup")
        currentFolder: pixelStudioDataPath
        onAccepted: {
            if (converter.importSettingsFrom(selectedFolder))
                win.statusMessage(qsTr("Settings imported — restart app"))
            else
                win.statusMessage(qsTr("Settings import failed"))
        }
    }

    FileDialog {
        id: batchSaveDialog
        title: qsTr("Save batch header")
        fileMode: FileDialog.SaveFile
        currentFolder: win.localFolderUrl(converter.lastExportDir, pixelStudioExportsUrl)
        defaultSuffix: "h"
        nameFilters: [ qsTr("C header") + " (*.h)" ]
        onAccepted: {
            converter.enqueueBatchCodeExport(win.pendingBatchFiles, selectedFile)
            win.statusMessage(qsTr("Batch queued"))
        }
    }

    AboutDialog {
        id: aboutDialog
        parent: win.overlay
        anchors.centerIn: parent
        studio: root.studio
        bodyWidth: win.dialogBodyWidth(440, root.studio.spacingXl * 4)
    }

    CodeSyntaxColorsDialog {
        id: syntaxColorsDialog
        parent: win.overlay
        anchors.centerIn: parent
        studio: root.studio
    }

    Dialog {
        id: resetSessionConfirmDialog
        parent: win.overlay
        anchors.centerIn: parent
        title: qsTr("Reset session?")
        modal: true
        standardButtons: Dialog.Yes | Dialog.No
        padding: root.studio.spacingLg
        property int bodyWidth: win.dialogBodyWidth(320, root.studio.spacingXl * 4)
        width: bodyWidth + 2 * padding
        onAccepted: {
            converter.resetSession()
            win.statusMessage(qsTr("Session reset"))
        }
        background: Rectangle {
            radius: root.studio.radiusMd
            color: root.studio.surface
            border.width: 2
            border.color: root.studio.accent
        }
        contentItem: Label {
            width: resetSessionConfirmDialog.bodyWidth
            wrapMode: Text.WordWrap
            text: qsTr("Clears inspector settings, recent lists, and watch folders. Projects on disk are not deleted.")
            font.family: root.studio.fontFamily
            color: root.studio.text
        }
    }

    Dialog {
        id: closeTabConfirmDialog
        parent: win.overlay
        anchors.centerIn: parent
        title: qsTr("Close tab?")
        modal: true
        standardButtons: Dialog.Yes | Dialog.No
        padding: root.studio.spacingLg
        property int bodyWidth: win.dialogBodyWidth(360, root.studio.spacingXl * 4)
        width: bodyWidth + 2 * padding
        onAccepted: {
            if (closeTabDontAsk.checked)
                appSettings.setConfirmCloseTab(false)
            if (win.pendingCloseTabId.length > 0)
                tabController.closeTab(win.pendingCloseTabId)
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
                    .arg(win.tabTitleForId(win.pendingCloseTabId))
                font.family: root.studio.fontFamily
                color: root.studio.text
            }
            StudioCheck {
                id: closeTabDontAsk
                studio: root.studio
                text: qsTr("Don't ask again")
            }
        }
    }

    Dialog {
        id: exitConfirmDialog
        parent: win.overlay
        anchors.centerIn: parent
        title: qsTr("Exit PixelStudio?")
        modal: true
        standardButtons: Dialog.Yes | Dialog.No
        padding: root.studio.spacingLg
        property int bodyWidth: win.dialogBodyWidth(360, root.studio.spacingXl * 4)
        width: bodyWidth + 2 * padding
        onAccepted: {
            win.forceClose = true
            win.close()
        }
        background: Rectangle {
            radius: root.studio.radiusMd
            color: root.studio.surface
            border.width: 2
            border.color: root.studio.accent
        }
        contentItem: Label {
            width: exitConfirmDialog.bodyWidth
            wrapMode: Text.WordWrap
            text: qsTr("Close the application? Unsaved exported files or project changes may be lost.")
            font.family: root.studio.fontFamily
            color: root.studio.text
        }
    }

}
