pragma Singleton

import QtQuick

pragma Translator: Shell


QtObject {
    id: root

    property var window: null
    property var dialogs: null
    property var layout: null
    property var toast: null
    property var toastTimer: null
    property int settingsSection: 0

    function bind(shell) {
        window = shell.window
        dialogs = shell.dialogs
        layout = shell.layout
        toast = shell.toast
        toastTimer = shell.toastTimer
    }

    function shellTr(source) {
        void workspace.settings.languageCode
        return qsTranslate("Shell", source)
    }

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

    function dialogBodyWidth(preferred, outerMargin) {
        return Math.max(240, Math.min(preferred, window.width - outerMargin))
    }

    function openWelcome() {
        workspace.tabs.activateWelcome()
    }

    function openSettingsTab(section) {
        if (section !== undefined)
            settingsSection = section
        workspace.tabs.openSettingsTab()
    }

    function ensureStudioTab() {
        if (!workspace.tabs.activeIsWelcome && !workspace.tabs.activeIsSettings)
            return
        const tabs = workspace.tabs.tabs
        for (let i = 0; i < tabs.length; ++i) {
            if (!tabs[i].isWelcome) {
                workspace.tabs.activateTab(tabs[i].id)
                return
            }
        }
        workspace.tabs.newProjectTab(qsTr("Untitled"))
    }

    function setWorkspaceView(mode) {
        ensureStudioTab()
        workspace.tabs.activeViewMode = mode
    }

    function setViewDual() { setWorkspaceView(StudioViewMode.Dual) }
    function setViewSource() { setWorkspaceView(StudioViewMode.Source) }
    function setViewOutput() { setWorkspaceView(StudioViewMode.Output) }

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
        workspace.tabs.openFileTab(local)
    }

    function tabTitleForId(tabId) {
        const tabs = workspace.tabs.tabs
        for (let i = 0; i < tabs.length; ++i) {
            if (tabs[i].id === tabId)
                return tabs[i].title
        }
        return ""
    }

    function requestCloseTab(tabId) {
        if (!tabId || tabId.length === 0)
            return
        if (!workspace.settings.confirmCloseTab) {
            workspace.tabs.closeTab(tabId)
            return
        }
        window.pendingCloseTabId = tabId
        dialogs.closeTabConfirmDialog.open()
    }

    function openDroppedUrls(urls) {
        if (!urls || urls.length === 0)
            return
        openLocalPath(urls[0])
    }

    function importClipboard() {
        if (workspace.tabs.activeIsWelcome || workspace.tabs.activeIsSettings)
            workspace.tabs.newProjectTab(qsTr("Untitled"))
        if (workspace.image.loadFromClipboard())
            workspace.tabs.syncActiveTabTitle()
    }

    function saveProject() {
        if (!workspace.project.saveProject())
            dialogs.saveProjectDialog.open()
    }

    function saveProjectAs() {
        dialogs.saveProjectDialog.open()
    }

    function saveCode() {
        dialogs.saveCodeDialog.open()
    }

    function saveBin() {
        dialogs.saveBinDialog.open()
    }

    function batchExport() {
        dialogs.batchOpenDialog.open()
    }

    function buildAtlas() {
        dialogs.atlasOpenDialog.open()
    }

    function openExportHub() {
        if (workspace.tabs.activeIsWelcome || workspace.tabs.activeIsSettings)
            workspace.tabs.newProjectTab(qsTr("Untitled"))
        if (workspace.settings.showSidebar && layout)
            layout.inspectorPageIndex = 2
    }

    function importHeader() {
        dialogs.importHeaderDialog.open()
    }

    function configureWatchFolder() {
        dialogs.watchInputDialog.open()
    }

    function togglePixelGrid() {
        workspace.viewport.setShowGrid(!workspace.viewport.showGrid)
        workspace.settings.setShowPixelGrid(workspace.viewport.showGrid)
    }

    function statusMessage(msg) {
        if (!toast)
            return
        toast.visible = msg.length > 0
        toast.text = msg
        if (toastTimer)
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
        const languages = workspace.settings.availableLanguages()
        for (let i = 0; i < languages.length; ++i) {
            if (languages[i].code === workspace.settings.languageCode)
                return i
        }
        return 0
    }

    function run(commandId) {
        switch (commandId) {
        case "welcome": openWelcome(); break
        case "newProject": workspace.tabs.newProjectTab(qsTr("Untitled")); break
        case "openProject": dialogs.openProjectDialog.open(); break
        case "saveProject": saveProject(); break
        case "saveProjectAs": saveProjectAs(); break
        case "openImage": dialogs.openDialog.open(); break
        case "importHeader": importHeader(); break
        case "importClipboard": importClipboard(); break
        case "importSettings": dialogs.settingsImportDialog.open(); break
        case "exportHub": openExportHub(); break
        case "saveCode": saveCode(); break
        case "saveBin": saveBin(); break
        case "batchExport": batchExport(); break
        case "buildAtlas": buildAtlas(); break
        case "exportSettings": dialogs.settingsExportDialog.open(); break
        case "clearProject": workspace.image.clear(); break
        case "exit": window.close(); break
        case "copyCode":
            workspace.exportPanel.copyToClipboard(workspace.image.generatedCode)
            statusMessage(qsTr("Copied to clipboard"))
            break
        case "clearImage": workspace.image.clear(); break
        case "toggleSidebar": workspace.settings.setShowSidebar(!workspace.settings.showSidebar); break
        case "viewDual": setViewDual(); break
        case "viewSource": setViewSource(); break
        case "viewOutput": setViewOutput(); break
        case "toggleCodeWrap": workspace.settings.setCodeWrap(!workspace.settings.codeWrap); break
        case "toggleGrid": togglePixelGrid(); break
        case "rotateCw": workspace.transform.rotateClockwise(); break
        case "flipH": workspace.transform.setFlipHorizontal(!workspace.transform.flipHorizontal); break
        case "flipV": workspace.transform.setFlipVertical(!workspace.transform.flipVertical); break
        case "invertColors":
            if (workspace.output.encodingIsMono1Bit)
                workspace.transform.setInvertMono(!workspace.transform.invertMono)
            else
                workspace.filters.setFilterInvert(!workspace.filters.filterInvert)
            break
        case "swapDisplay": workspace.output.swapDisplayDimensions(); break
        case "watchFolder": configureWatchFolder(); break
        case "toggleWatch": workspace.exportPanel.setWatchFolderActive(!workspace.exportPanel.watchFolderActive); break
        case "preferences": openSettingsTab(0); break
        case "resetUi": workspace.settings.resetUiDefaults(); break
        case "openSettingsFolder": workspace.project.openApplicationDataFolder(); break
        case "resetSession": dialogs.resetSessionConfirmDialog.open(); break
        case "openDocumentsFolder": workspace.project.openUserDocumentsFolder(); break
        case "about": dialogs.aboutDialog.open(); break
        }
    }

    function commandEnabled(commandId) {
        switch (commandId) {
        case "saveCode":
        case "copyCode":
            return workspace.image.generatedCode.length > 0
        case "saveBin":
            return workspace.image.hasPreview
        case "clearProject":
        case "clearImage":
            return workspace.image.hasImage
        case "rotateCw":
        case "flipH":
        case "flipV":
        case "invertColors":
        case "swapDisplay":
            return workspace.image.hasImage
        default:
            return true
        }
    }

    function commandChecked(commandId) {
        switch (commandId) {
        case "toggleSidebar":
            return workspace.settings.showSidebar
        case "toggleCodeWrap":
            return workspace.settings.codeWrap
        case "toggleGrid":
            return workspace.viewport.showGrid
        case "flipH":
            return workspace.transform.flipHorizontal
        case "flipV":
            return workspace.transform.flipVertical
        case "invertColors":
            return workspace.output.encodingIsMono1Bit
                   ? workspace.transform.invertMono
                   : workspace.filters.filterInvert
        case "toggleWatch":
            return workspace.exportPanel.watchFolderActive
        default:
            return false
        }
    }
}
