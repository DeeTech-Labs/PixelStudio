import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import PixelStudio

pragma Translator: Shell


ApplicationWindow {
    id: window
    width: 1200
    height: 780
    minimumWidth: 960
    minimumHeight: 600
    visible: true

    Theme { id: appPalette }

    FontLoader {
        id: pixelFontLoader
        source: "qrc:/fonts/PressStart2P-Regular.ttf"
        onStatusChanged: {
            if (status === FontLoader.Ready)
                appPalette.fontFamilyPixel = name
        }
    }

    font.family: appPalette.fontFamily
    font.pixelSize: appPalette.fontSizeBase
    color: appPalette.background

    background: Rectangle {
        color: appPalette.background
    }

    title: workspace.tabs.activeIsWelcome
        ? qsTr("PixelStudio")
        : qsTr("PixelStudio — %1").arg(workspace.tabs.activeTabTitle)
    property var pendingBatchFiles: []
    property string pendingCloseTabId: ""
    property var pendingAtlasFiles: []
    property bool forceClose: false
    property int folderPickTarget: 0

    QtObject {
        id: toast
        property string text: ""
        property bool visible: false
    }

    Timer {
        id: toastTimer
        interval: 2600
        onTriggered: toast.visible = false
    }

    function stripMenuMnemonic(text) { return WorkflowRouter.stripMenuMnemonic(text) }
    function dialogBodyWidth(preferred, outerMargin) { return WorkflowRouter.dialogBodyWidth(preferred, outerMargin) }
    function openWelcome() { WorkflowRouter.openWelcome() }
    function setWorkspaceView(mode) { WorkflowRouter.setWorkspaceView(mode) }
    function openLocalPath(path) { WorkflowRouter.openLocalPath(path) }
    function tabTitleForId(tabId) { return WorkflowRouter.tabTitleForId(tabId) }
    function requestCloseTab(tabId) { WorkflowRouter.requestCloseTab(tabId) }
    function openDroppedUrls(urls) { WorkflowRouter.openDroppedUrls(urls) }
    function importClipboard() { WorkflowRouter.importClipboard() }
    function saveProject() { WorkflowRouter.saveProject() }
    function saveProjectAs() { WorkflowRouter.saveProjectAs() }
    function saveCode() { WorkflowRouter.saveCode() }
    function saveBin() { WorkflowRouter.saveBin() }
    function batchExport() { WorkflowRouter.batchExport() }
    function buildAtlas() { WorkflowRouter.buildAtlas() }
    function openExportHub() { WorkflowRouter.openExportHub() }
    function importHeader() { WorkflowRouter.importHeader() }
    function configureWatchFolder() { WorkflowRouter.configureWatchFolder() }
    function togglePixelGrid() { WorkflowRouter.togglePixelGrid() }
    function statusMessage(msg) { WorkflowRouter.statusMessage(msg) }
    function localFolderUrl(path, fallbackUrl) { return WorkflowRouter.localFolderUrl(path, fallbackUrl) }
    function languageIndex() { return WorkflowRouter.languageIndex() }

    Component.onCompleted: {
        WorkflowRouter.bind({
            window: window,
            dialogs: mainDialogs,
            layout: studioLayout,
            toast: toast,
            toastTimer: toastTimer
        })
    }

    onClosing: (close) => {
        if (workspace.settings.confirmExit && !forceClose) {
            close.accepted = false
            mainDialogs.exitConfirmDialog.open()
        }
    }

    menuBar: MainMenuBar {
        studio: appPalette
    }

    DropArea {
        anchors.fill: parent
        z: -1
        onDropped: (drop) => {
            if (drop.hasUrls && drop.urls.length > 0)
                workspace.image.loadImage(drop.urls[0])
        }
    }

    Connections {
        target: workspace.image
        function onErrorOccurred(message) { WorkflowRouter.statusMessage(message) }
    }

    Connections {
        target: workspace.tabs
        function onTabActionFailed(message) { WorkflowRouter.statusMessage(message) }
    }

    MainDialogs {
        id: mainDialogs
        anchors.fill: parent
        win: window
        studio: appPalette
    }

    readonly property string statusSpecs: {
        if (!workspace.image.hasPreview)
            return ""
        return workspace.output.displayWidth + " × " + workspace.output.displayHeight
            + "  " + workspace.output.encodingModeName
            + "  " + workspace.image.previewColorCount + " " + qsTr("Colors")
    }

    StudioShell {
        id: studioShell
        anchors.fill: parent
        studio: appPalette
        statusText: toast.text.length > 0 ? toast.text : qsTr("Ready")
        specsText: statusSpecs
        toastText: toast.visible ? toast.text : ""

        SettingsPage {
            anchors.fill: parent
            visible: workspace.tabs.activeIsSettings
            studio: appPalette
            win: window
            hostDialogs: mainDialogs
        }

        WelcomeScreen {
            anchors.fill: parent
            visible: workspace.tabs.activeIsWelcome
            studio: appPalette
            onOpenImageRequested: WorkflowRouter.run("openImage")
            onPasteRequested: WorkflowRouter.importClipboard()
            onOpenProjectRequested: WorkflowRouter.run("openProject")
            onNewProjectRequested: WorkflowRouter.run("newProject")
            onContinueLastProjectRequested: {
                if (workspace.project.hasRestorableProject)
                    workspace.tabs.openProjectTab(WorkflowRouter.localFolderUrl(
                        workspace.project.lastProjectPath, workspace.settings.projectsUrl))
            }
            onRecentItemRequested: (path, tabId) => {
                if (tabId && tabId.length > 0 && workspace.tabs.activateTab(tabId))
                    return
                WorkflowRouter.openLocalPath(path)
            }
            onFileDropped: (urls) => WorkflowRouter.openDroppedUrls(urls)
            onSettingsRequested: WorkflowRouter.run("preferences")
        }

        StudioLayout {
            id: studioLayout
            anchors.fill: parent
            visible: !workspace.tabs.activeIsWelcome && !workspace.tabs.activeIsSettings
            studio: appPalette
            viewMode: workspace.tabs.activeViewMode
            win: window
            onOpenRequested: function() { WorkflowRouter.run("openImage") }
            onPasteRequested: WorkflowRouter.importClipboard()
            onViewModeRequested: (mode) => WorkflowRouter.setWorkspaceView(mode)
            onAssetOpenRequested: (path) => WorkflowRouter.openLocalPath(path)
            onSaveCodeRequested: WorkflowRouter.saveCode()
            onSaveBinRequested: WorkflowRouter.saveBin()
            onNotify: (msg) => WorkflowRouter.statusMessage(msg)
        }
    }
}
