import QtQuick
import QtQuick.Dialogs
import PixelStudio

pragma Translator: Shell


Item {
    id: root

    required property var win
    required property var studio

    property alias openDialog: openDialog
    property alias openProjectDialog: openProjectDialog
    property alias saveProjectDialog: saveProjectDialog
    property alias importHeaderDialog: importHeaderDialog

    FileDialog {
        id: openDialog
        objectName: "openImageDialog"
        title: qsTr("Open image")
        currentFolder: WorkflowRouter.localFolderUrl(workspace.project.lastOpenImageDir, workspace.settings.projectsUrl)
        nameFilters: [
            qsTr("Images") + " (*.png *.jpg *.jpeg *.bmp *.gif *.webp)",
            qsTr("All files") + " (*)"
        ]
        onAccepted: WorkflowRouter.openLocalPath(selectedFile)
    }

    FileDialog {
        id: openProjectDialog
        objectName: "openProjectDialog"
        title: qsTr("Open PixelStudio project")
        currentFolder: workspace.settings.projectsUrl
        nameFilters: [ qsTr("PixelStudio project") + " (*.pspx)", qsTr("All files") + " (*)" ]
        onAccepted: workspace.tabs.openProjectTab(selectedFile)
    }

    FileDialog {
        id: saveProjectDialog
        objectName: "saveProjectDialog"
        title: qsTr("Save PixelStudio project")
        fileMode: FileDialog.SaveFile
        currentFolder: workspace.settings.projectsUrl
        defaultSuffix: "pspx"
        nameFilters: [ qsTr("PixelStudio project") + " (*.pspx)" ]
        onAccepted: {
            if (workspace.project.saveProjectAs(selectedFile)) {
                workspace.tabs.syncActiveTabTitle()
                WorkflowRouter.statusMessage(qsTr("Project saved"))
            }
        }
    }

    FileDialog {
        id: importHeaderDialog
        title: qsTr("Import C header")
        currentFolder: WorkflowRouter.localFolderUrl(workspace.project.lastOpenImageDir, workspace.settings.exportsUrl)
        nameFilters: [ qsTr("C header") + " (*.h *.hpp)", qsTr("All files") + " (*)" ]
        onAccepted: {
            if (workspace.tabs.activeIsWelcome || workspace.tabs.activeIsSettings)
                workspace.tabs.newProjectTab(qsTr("Untitled"))
            if (workspace.project.importHeader(selectedFile))
                workspace.tabs.syncActiveTabTitle()
        }
    }
}
