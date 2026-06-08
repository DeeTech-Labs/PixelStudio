import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import PixelStudio

pragma Translator: PixelStudio

ApplicationWindow {
    id: window
    width: 1200
    height: 780

    function stripMenuMnemonic(text) {
        let out = ""
        for (let i = 0; i < text.length; ++i) {
            const ch = text[i]
            if (ch === "&") {
                if (i + 1 < text.length && text[i + 1] === "&") {
                    out += "&"
                    ++i
                } else if (i + 1 < text.length) {
                    out += text[++i]
                }
            } else {
                out += ch
            }
        }
        return out
    }
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

    title: tabController.activeIsWelcome
        ? qsTr("PixelStudio")
        : qsTr("PixelStudio — %1").arg(tabController.activeTabTitle)
    property var pendingBatchFiles: []
    property string pendingCloseTabId: ""
    property var pendingAtlasFiles: []
    property bool forceClose: false
    property int folderPickTarget: 0

    function dialogBodyWidth(preferred, outerMargin) {
        return Math.max(240, Math.min(preferred, width - outerMargin))
    }

    function openWelcome() {
        tabController.activateWelcome()
    }

    function ensureStudioTab() {
        if (!tabController.activeIsWelcome)
            return
        const tabs = tabController.tabs
        for (let i = 0; i < tabs.length; ++i) {
            if (!tabs[i].isWelcome) {
                tabController.activateTab(tabs[i].id)
                return
            }
        }
        tabController.newProjectTab(qsTr("Untitled"))
    }

    function setWorkspaceView(mode) {
        ensureStudioTab()
        tabController.activeViewMode = mode
    }

    function urlToLocalPath(url) {
        if (!url)
            return ""
        if (typeof url === "string")
            url = Qt.resolvedUrl(url)
        const local = url.toLocalFile !== undefined ? url.toLocalFile() : ""
        if (local && local.length > 0)
            return local
        let s = url.toString()
        if (s.startsWith("file:///"))
            s = s.slice(8)
        else if (s.startsWith("file://"))
            s = s.slice(7)
        try {
            return decodeURIComponent(s)
        } catch (e) {
            return s
        }
    }

    function openLocalPath(path) {
        const local = urlToLocalPath(path)
        if (!local || local.length === 0)
            return
        tabController.openFileTab(local)
    }

    function tabTitleForId(tabId) {
        const tabs = tabController.tabs
        for (let i = 0; i < tabs.length; ++i) {
            if (tabs[i].id === tabId)
                return tabs[i].title
        }
        return ""
    }

    function requestCloseTab(tabId) {
        if (!tabId || tabId.length === 0)
            return
        if (!appSettings.confirmCloseTab) {
            tabController.closeTab(tabId)
            return
        }
        pendingCloseTabId = tabId
        mainDialogs.closeTabConfirmDialog.open()
    }

    function openDroppedUrls(urls) {
        if (!urls || urls.length === 0)
            return
        openLocalPath(urls[0])
    }

    function importClipboard() {
        if (tabController.activeIsWelcome)
            tabController.newProjectTab(qsTr("Untitled"))
        if (converter.loadFromClipboard())
            tabController.syncActiveTabTitle()
    }

    Connections {
        target: tabController
        function onTabActionFailed(message) { statusMessage(message) }
    }

    QtObject {
        id: exportBridge
        function notify(msg) { statusMessage(msg) }
        function saveProject() {
            if (!converter.saveProject())
                mainDialogs.saveProjectDialog.open()
        }
        function saveProjectAs() { mainDialogs.saveProjectDialog.open() }
        function saveCode() { mainDialogs.saveCodeDialog.open() }
        function saveBin() { mainDialogs.saveBinDialog.open() }
        function batchExport() { mainDialogs.batchOpenDialog.open() }
        function buildAtlas() { mainDialogs.atlasOpenDialog.open() }
        function openExportHub() {
            if (tabController.activeIsWelcome)
                tabController.newProjectTab(qsTr("Untitled"))
            if (appSettings.showSidebar)
                studioLayout.inspectorPageIndex = 2
        }
        function importHeader() { mainDialogs.importHeaderDialog.open() }
        function configureWatchFolder() { mainDialogs.watchInputDialog.open() }
    }

    function statusMessage(msg) {
        toast.visible = msg.length > 0
        toast.text = msg
        toastTimer.restart()
    }

    function localFolderUrl(path, fallbackUrl) {
        if (!path || path.length === 0)
            return fallbackUrl
        const normalized = path.replace(/\\/g, "/")
        return normalized.startsWith("/")
            ? ("file://" + normalized)
            : ("file:///" + normalized)
    }

    function languageIndex() {
        const languages = appSettings.availableLanguages()
        for (let i = 0; i < languages.length; ++i) {
            if (languages[i].code === appSettings.languageCode)
                return i
        }
        return 0
    }

    onClosing: (close) => {
        if (appSettings.confirmExit && !forceClose) {
            close.accepted = false
            mainDialogs.exitConfirmDialog.open()
        }
    }

    menuBar: MainMenuBar {
        win: window
        dialogs: mainDialogs
        studio: appPalette
    }

    DropArea {
        anchors.fill: parent
        z: -1
        onDropped: (drop) => {
            if (drop.hasUrls && drop.urls.length > 0)
                converter.loadImage(drop.urls[0])
        }
    }

    Connections {
        target: converter
        function onErrorOccurred(message) { statusMessage(message) }
    }

    MainDialogs {
        id: mainDialogs
        anchors.fill: parent
        win: window
        studio: appPalette
    }

    readonly property string statusSpecs: {
        if (!converter.hasPreview)
            return ""
        return converter.displayWidth + " × " + converter.displayHeight
            + "  " + converter.encodingModeName
            + "  " + converter.previewColorCount + " " + qsTr("Colors")
    }

    StudioShell {
        id: studioShell
        anchors.fill: parent
        studio: appPalette
        statusText: toast.text.length > 0 ? toast.text : qsTr("Ready")
        specsText: statusSpecs
        toastText: toast.visible ? toast.text : ""

        onCloseTabRequested: (tabId) => requestCloseTab(tabId)
        onOpenImageTabRequested: mainDialogs.openDialog.open()
        onSettingsRequested: mainDialogs.preferencesDialog.open()
        onNewProjectRequested: tabController.newProjectTab(qsTr("Untitled"))
        onOpenRequested: mainDialogs.openDialog.open()
        onSaveRequested: exportBridge.saveProject()
        onToggleGridRequested: appSettings.setShowPixelGrid(!appSettings.showPixelGrid)
        onViewDualRequested: setWorkspaceView(0)
        onViewSourceRequested: setWorkspaceView(1)
        onViewOutputRequested: setWorkspaceView(3)

        WelcomeScreen {
            anchors.fill: parent
            visible: tabController.activeIsWelcome
            studio: appPalette
            onOpenImageRequested: mainDialogs.openDialog.open()
            onPasteRequested: importClipboard()
            onOpenProjectRequested: mainDialogs.openProjectDialog.open()
            onNewProjectRequested: tabController.newProjectTab(qsTr("Untitled"))
            onContinueLastProjectRequested: {
                if (converter.hasRestorableProject)
                    tabController.openProjectTab(localFolderUrl(converter.lastProjectPath, pixelStudioProjectsUrl))
            }
            onRecentItemRequested: (path, tabId) => {
                if (tabId && tabId.length > 0 && tabController.activateTab(tabId))
                    return
                openLocalPath(path)
            }
            onFileDropped: (urls) => openDroppedUrls(urls)
            onSettingsRequested: mainDialogs.preferencesDialog.open()
        }

        StudioLayout {
            id: studioLayout
            anchors.fill: parent
            visible: !tabController.activeIsWelcome
            studio: appPalette
            viewMode: tabController.activeViewMode
            exportBridge: exportBridge
            onOpenRequested: function() { mainDialogs.openDialog.open() }
            onPasteRequested: importClipboard()
            onViewModeRequested: (mode) => setWorkspaceView(mode)
            onAssetOpenRequested: (path) => openLocalPath(path)
            onSaveCodeRequested: mainDialogs.saveCodeDialog.open()
            onSaveBinRequested: mainDialogs.saveBinDialog.open()
            onNotify: (msg) => statusMessage(msg)
        }
    }

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
}
