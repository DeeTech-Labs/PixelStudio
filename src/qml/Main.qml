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
        closeTabConfirmDialog.open()
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
                saveProjectDialog.open()
        }
        function saveProjectAs() { saveProjectDialog.open() }
        function saveCode() { saveCodeDialog.open() }
        function saveBin() { saveBinDialog.open() }
        function batchExport() { batchOpenDialog.open() }
        function buildAtlas() { atlasOpenDialog.open() }
        function openExportHub() {
            if (tabController.activeIsWelcome)
                tabController.newProjectTab(qsTr("Untitled"))
            if (appSettings.showSidebar)
                studioLayout.inspectorPageIndex = 2
        }
        function importHeader() { importHeaderDialog.open() }
        function configureWatchFolder() { watchInputDialog.open() }
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
            exitConfirmDialog.open()
        }
    }

    menuBar: MenuBar {
        background: Rectangle {
            color: appPalette.surface
            border.width: 1
            border.color: appPalette.border
            implicitHeight: appPalette.menuBarHeight
        }
        delegate: MenuBarItem {
            id: menuBarItem
            contentItem: Label {
                text: window.stripMenuMnemonic(menuBarItem.text).toUpperCase()
                font.family: appPalette.fontFamilyPixel
                font.pixelSize: appPalette.fontSizePixel
                color: menuBarItem.highlighted ? appPalette.accent : appPalette.textSecondary
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            background: Rectangle {
                radius: appPalette.radiusSm
                color: menuBarItem.highlighted ? appPalette.railHover : "transparent"
            }
        }
        Menu {
            title: qsTr("&File")
            Action {
                text: qsTr("Welcome screen")
                onTriggered: openWelcome()
            }
            MenuSeparator {}
            Action {
                text: qsTr("New project")
                shortcut: "Ctrl+N"
                onTriggered: tabController.newProjectTab(qsTr("Untitled"))
            }
            Action {
                text: qsTr("Open project…")
                shortcut: "Ctrl+Shift+O"
                onTriggered: openProjectDialog.open()
            }
            Action {
                text: qsTr("Save project")
                shortcut: "Ctrl+Shift+S"
                onTriggered: {
                    if (!converter.saveProject())
                        saveProjectDialog.open()
                }
            }
            Action {
                text: qsTr("Save project as…")
                onTriggered: saveProjectDialog.open()
            }
            MenuSeparator {}
            Menu {
                id: menuBarImportMenu
                title: qsTr("Import")
                Action {
                    text: qsTr("Open image…")
                    shortcut: "Ctrl+O"
                    onTriggered: openDialog.open()
                }
                Action {
                    text: qsTr("Open project…")
                    shortcut: "Ctrl+Shift+O"
                    onTriggered: openProjectDialog.open()
                }
                Action {
                    text: qsTr("Import C header…")
                    onTriggered: importHeaderDialog.open()
                }
                Action {
                    text: qsTr("Paste from clipboard")
                    shortcut: "Ctrl+V"
                    onTriggered: importClipboard()
                }
                MenuSeparator {}
                Menu {
                    id: openRecentMenu
                    title: qsTr("Open recent")
                    enabled: converter.recentFiles.length > 0
                    Instantiator {
                        model: converter.recentFiles
                        delegate: MenuItem {
                            required property var modelData
                            text: modelData.name
                            onTriggered: openLocalPath(modelData.path)
                        }
                        onObjectAdded: (index, object) => openRecentMenu.insertItem(index, object)
                        onObjectRemoved: (index, object) => openRecentMenu.removeItem(object)
                    }
                }
                MenuSeparator {}
                Action {
                    text: qsTr("Import settings backup…")
                    onTriggered: settingsImportDialog.open()
                }
            }
            Menu {
                id: menuBarExportMenu
                title: qsTr("Export")
                Action {
                    text: qsTr("Export panel…")
                    shortcut: "Ctrl+Shift+E"
                    onTriggered: exportBridge.openExportHub()
                }
                MenuSeparator {}
                Action {
                    text: qsTr("Save project")
                    shortcut: "Ctrl+Shift+S"
                    onTriggered: exportBridge.saveProject()
                }
                Action {
                    text: qsTr("Save project as…")
                    onTriggered: exportBridge.saveProjectAs()
                }
                MenuSeparator {}
                Action {
                    text: qsTr("Save code to file…")
                    shortcut: "Ctrl+Alt+S"
                    enabled: converter.generatedCode.length > 0
                    onTriggered: exportBridge.saveCode()
                }
                Action {
                    text: qsTr("Save binary…")
                    enabled: converter.hasPreview
                    onTriggered: exportBridge.saveBin()
                }
                MenuSeparator {}
                Action {
                    text: qsTr("Batch export .h…")
                    shortcut: "Ctrl+Shift+B"
                    onTriggered: exportBridge.batchExport()
                }
                Action {
                    text: qsTr("Build sprite atlas…")
                    onTriggered: exportBridge.buildAtlas()
                }
                MenuSeparator {}
                Menu {
                    id: menuBarRecentExportsMenu
                    title: qsTr("Recent exports")
                    enabled: converter.recentExports.length > 0
                    Instantiator {
                        model: converter.recentExports
                        delegate: MenuItem {
                            required property var modelData
                            text: modelData.name
                            onTriggered: Qt.openUrlExternally(modelData.path)
                        }
                        onObjectAdded: (index, object) => menuBarRecentExportsMenu.insertItem(index, object)
                        onObjectRemoved: (index, object) => menuBarRecentExportsMenu.removeItem(object)
                    }
                }
                MenuSeparator {}
                Action {
                    text: qsTr("Export settings backup…")
                    onTriggered: settingsExportDialog.open()
                }
            }
            MenuSeparator {}
            Action {
                text: qsTr("&Clear project")
                enabled: converter.hasImage
                onTriggered: converter.clear()
            }
            MenuSeparator {}
            Action {
                text: qsTr("E&xit")
                shortcut: "Ctrl+Q"
                onTriggered: window.close()
            }
        }
        Menu {
            title: qsTr("&Edit")
            Action {
                text: qsTr("&Copy generated code")
                shortcut: "Ctrl+C"
                enabled: converter.generatedCode.length > 0
                onTriggered: {
                    converter.copyToClipboard(converter.generatedCode)
                    statusMessage(qsTr("Copied to clipboard"))
                }
            }
            MenuSeparator {}
            Action {
                text: qsTr("Clear image")
                enabled: converter.hasImage
                onTriggered: converter.clear()
            }
        }
        Menu {
            title: qsTr("&View")
            Action {
                text: qsTr("Inspector panel")
                checkable: true
                checked: appSettings.showSidebar
                shortcut: "Ctrl+B"
                onTriggered: appSettings.setShowSidebar(!appSettings.showSidebar)
            }
            Action {
                text: qsTr("View: Dual")
                onTriggered: setWorkspaceView(0)
            }
            Action {
                text: qsTr("View: Source")
                onTriggered: setWorkspaceView(1)
            }
            Action {
                text: qsTr("View: Output")
                onTriggered: setWorkspaceView(3)
            }
            Action {
                text: qsTr("Wrap generated code")
                checkable: true
                checked: appSettings.codeWrap
                onTriggered: appSettings.setCodeWrap(!appSettings.codeWrap)
            }
            Action {
                text: qsTr("Show pixel grid")
                checkable: true
                checked: appSettings.showPixelGrid
                onTriggered: appSettings.setShowPixelGrid(!appSettings.showPixelGrid)
            }
        }
        Menu {
            title: qsTr("&Tools")
            Action {
                text: qsTr("Batch export .h…")
                shortcut: "Ctrl+Shift+B"
                onTriggered: batchOpenDialog.open()
            }
            Action {
                text: qsTr("Build sprite atlas…")
                onTriggered: atlasOpenDialog.open()
            }
            Action {
                text: qsTr("Import C header…")
                onTriggered: importHeaderDialog.open()
            }
            Action {
                text: qsTr("Configure watch folder…")
                onTriggered: watchInputDialog.open()
            }
            Action {
                text: qsTr("Watch folder")
                checkable: true
                checked: converter.watchFolderActive
                onTriggered: converter.setWatchFolderActive(!converter.watchFolderActive)
            }
        }
        Menu {
            title: qsTr("&Settings")
            Action {
                text: qsTr("Preferences…")
                shortcut: "Ctrl+,"
                onTriggered: preferencesDialog.open()
            }
            Menu {
                id: languageMenu
                title: qsTr("Language")
                Instantiator {
                    model: appSettings.availableLanguages()
                    delegate: MenuItem {
                        required property var modelData
                        text: modelData.name
                        checkable: true
                        checked: appSettings.languageCode === modelData.code
                        onTriggered: appSettings.setLanguageCode(modelData.code)
                    }
                    onObjectAdded: (index, object) => languageMenu.insertItem(index, object)
                    onObjectRemoved: (index, object) => languageMenu.removeItem(object)
                }
            }
            Action {
                text: qsTr("Reset UI defaults")
                onTriggered: appSettings.resetUiDefaults()
            }
            MenuSeparator {}
            Action {
                text: qsTr("Open settings folder")
                onTriggered: converter.openAppDataFolder()
            }
            Action {
                text: qsTr("Export settings backup…")
                onTriggered: settingsExportDialog.open()
            }
            Action {
                text: qsTr("Reset session")
                onTriggered: resetSessionConfirmDialog.open()
            }
            Menu {
                id: recentExportsMenu
                title: qsTr("Recent exports")
                enabled: converter.recentExports.length > 0
                Instantiator {
                    model: converter.recentExports
                    delegate: MenuItem {
                        required property var modelData
                        text: modelData.name
                        onTriggered: Qt.openUrlExternally(modelData.path)
                    }
                    onObjectAdded: (index, object) => recentExportsMenu.insertItem(index, object)
                    onObjectRemoved: (index, object) => recentExportsMenu.removeItem(object)
                }
            }
        }
        Menu {
            title: qsTr("&Image")
            enabled: converter.hasImage
            Action {
                text: qsTr("Rotate 90° clockwise")
                shortcut: "Ctrl+R"
                onTriggered: converter.rotateClockwise()
            }
            Action {
                text: qsTr("Flip horizontally")
                checkable: true
                checked: converter.flipHorizontal
                onTriggered: converter.setFlipHorizontal(!converter.flipHorizontal)
            }
            Action {
                text: qsTr("Flip vertically")
                checkable: true
                checked: converter.flipVertical
                onTriggered: converter.setFlipVertical(!converter.flipVertical)
            }
            Action {
                text: qsTr("Invert result colors")
                checkable: true
                checked: converter.encodingIsMono1Bit ? converter.invertMono : converter.filterInvert
                enabled: converter.hasImage
                onTriggered: {
                    if (converter.encodingIsMono1Bit)
                        converter.setInvertMono(!converter.invertMono)
                    else
                        converter.setFilterInvert(!converter.filterInvert)
                }
            }
            MenuSeparator {}
            Action {
                text: qsTr("Swap display width and height")
                onTriggered: converter.swapDisplayDimensions()
            }
        }
        Menu {
            title: qsTr("&Help")
            Action {
                text: qsTr("Open PixelStudio folder")
                onTriggered: converter.openUserDocumentsFolder()
            }
            MenuSeparator {}
            Action {
                text: qsTr("About PixelStudio")
                onTriggered: aboutDialog.open()
            }
        }
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

    FileDialog {
        id: openDialog
        title: qsTr("Open image")
        currentFolder: localFolderUrl(converter.lastOpenImageDir, pixelStudioProjectsUrl)
        nameFilters: [
            qsTr("Images") + " (*.png *.jpg *.jpeg *.bmp *.gif *.webp)",
            qsTr("All files") + " (*)"
        ]
        onAccepted: openLocalPath(selectedFile)
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
                statusMessage(qsTr("Project saved"))
            }
        }
    }

    FileDialog {
        id: importHeaderDialog
        title: qsTr("Import C header")
        currentFolder: localFolderUrl(converter.lastOpenImageDir, pixelStudioExportsUrl)
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
        currentFolder: localFolderUrl(converter.lastExportDir, pixelStudioExportsUrl)
        currentFile: converter.suggestedCodeFileUrl()
        defaultSuffix: "h"
        nameFilters: [
            qsTr("C header") + " (*.h)",
            qsTr("Text file") + " (*.txt)",
            qsTr("All files") + " (*)"
        ]
        onAccepted: {
            if (converter.saveCodeToFile(selectedFile))
                statusMessage(qsTr("File saved"))
        }
    }

    FileDialog {
        id: saveBinDialog
        title: qsTr("Save binary")
        fileMode: FileDialog.SaveFile
        currentFolder: localFolderUrl(converter.lastExportDir, pixelStudioExportsUrl)
        defaultSuffix: "bin"
        nameFilters: [
            qsTr("Binary") + " (*.bin)",
            qsTr("All files") + " (*)"
        ]
        onAccepted: {
            if (converter.saveBinaryToFile(selectedFile))
                statusMessage(qsTr("File saved"))
        }
    }

    FileDialog {
        id: batchOpenDialog
        title: qsTr("Select images for batch")
        fileMode: FileDialog.OpenFiles
        currentFolder: localFolderUrl(converter.lastOpenImageDir, pixelStudioProjectsUrl)
        nameFilters: [
            qsTr("Images") + " (*.png *.jpg *.jpeg *.bmp *.gif *.webp)",
            qsTr("All files") + " (*)"
        ]
        onAccepted: {
            pendingBatchFiles = selectedFiles
            if (pendingBatchFiles.length > 0)
                batchSaveDialog.open()
        }
    }

    FileDialog {
        id: atlasOpenDialog
        title: qsTr("Select images for atlas")
        fileMode: FileDialog.OpenFiles
        currentFolder: localFolderUrl(converter.lastOpenImageDir, pixelStudioProjectsUrl)
        nameFilters: [
            qsTr("Images") + " (*.png *.jpg *.jpeg *.bmp *.gif *.webp)",
            qsTr("All files") + " (*)"
        ]
        onAccepted: {
            pendingAtlasFiles = selectedFiles
            if (pendingAtlasFiles.length > 0)
                atlasSaveDialog.open()
        }
    }

    FileDialog {
        id: atlasSaveDialog
        title: qsTr("Save atlas header")
        fileMode: FileDialog.SaveFile
        currentFolder: localFolderUrl(converter.lastExportDir, pixelStudioExportsUrl)
        defaultSuffix: "h"
        nameFilters: [ qsTr("C header") + " (*.h)" ]
        onAccepted: {
            if (converter.buildSpriteAtlas(pendingAtlasFiles, selectedFile, converter.displayWidth, converter.displayHeight))
                statusMessage(qsTr("Atlas saved"))
        }
    }

    FolderDialog {
        id: customFolderDialog
        title: qsTr("Choose folder")
        onAccepted: {
            const path = selectedFolder.toLocalFile()
            if (folderPickTarget === 0)
                appSettings.setDocumentsRoot(path)
            else if (folderPickTarget === 1)
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
            statusMessage(qsTr("Watch folder enabled"))
        }
    }

    FolderDialog {
        id: settingsExportDialog
        title: qsTr("Export settings backup")
        currentFolder: pixelStudioDataPath
        onAccepted: {
            if (converter.exportSettingsTo(selectedFolder))
                statusMessage(qsTr("Settings exported"))
            else
                statusMessage(qsTr("Settings export failed"))
        }
    }

    FolderDialog {
        id: settingsImportDialog
        title: qsTr("Import settings backup")
        currentFolder: pixelStudioDataPath
        onAccepted: {
            if (converter.importSettingsFrom(selectedFolder))
                statusMessage(qsTr("Settings imported — restart app"))
            else
                statusMessage(qsTr("Settings import failed"))
        }
    }

    FileDialog {
        id: batchSaveDialog
        title: qsTr("Save batch header")
        fileMode: FileDialog.SaveFile
        currentFolder: localFolderUrl(converter.lastExportDir, pixelStudioExportsUrl)
        defaultSuffix: "h"
        nameFilters: [ qsTr("C header") + " (*.h)" ]
        onAccepted: {
            converter.enqueueBatchCodeExport(pendingBatchFiles, selectedFile)
            statusMessage(qsTr("Batch queued"))
        }
    }

    AboutDialog {
        id: aboutDialog
        studio: appPalette
        bodyWidth: window.dialogBodyWidth(440, appPalette.spacingXl * 4)
    }

    CodeSyntaxColorsDialog {
        id: syntaxColorsDialog
        studio: appPalette
    }

    Dialog {
        id: preferencesDialog
        title: qsTr("Preferences")
        modal: true
        anchors.centerIn: parent
        standardButtons: Dialog.Ok
        padding: appPalette.spacingLg
        property int bodyWidth: window.dialogBodyWidth(488, appPalette.spacingXl * 4)
        width: bodyWidth + 2 * padding

        background: Rectangle {
            radius: appPalette.radiusMd
            color: appPalette.surface
            border.width: 2
            border.color: appPalette.accent
        }

        ColumnLayout {
            width: preferencesDialog.bodyWidth
            spacing: appPalette.spacingMd

            StudioSection {
                Layout.fillWidth: true
                studio: appPalette
                title: qsTr("Application")
                hint: qsTr("Shell layout and language.")

                StudioField { studio: appPalette; labelText: qsTr("Language") }
                StudioCombo {
                    Layout.fillWidth: true
                    studio: appPalette
                    model: appSettings.availableLanguages()
                    textRole: "name"
                    currentIndex: languageIndex()
                    onActivated: {
                        const item = model[currentIndex]
                        if (item && item.code)
                            appSettings.setLanguageCode(item.code)
                    }
                }
                StudioCheck {
                    Layout.fillWidth: true
                    studio: appPalette
                    text: qsTr("Show inspector panel")
                    checked: appSettings.showSidebar
                    onToggled: appSettings.setShowSidebar(checked)
                }
                StudioCheck {
                    Layout.fillWidth: true
                    studio: appPalette
                    text: qsTr("Show pixel grid")
                    checked: appSettings.showPixelGrid
                    onToggled: appSettings.setShowPixelGrid(checked)
                }
                StudioCheck {
                    Layout.fillWidth: true
                    studio: appPalette
                    text: qsTr("Wrap generated code")
                    checked: appSettings.codeWrap
                    onToggled: appSettings.setCodeWrap(checked)
                }
                StudioButton {
                    Layout.fillWidth: true
                    studio: appPalette
                    text: qsTr("Code syntax colors…")
                    onClicked: syntaxColorsDialog.open()
                }
                StudioCheck {
                    Layout.fillWidth: true
                    studio: appPalette
                    text: qsTr("Confirm before exit")
                    checked: appSettings.confirmExit
                    onToggled: appSettings.setConfirmExit(checked)
                }
                StudioCheck {
                    Layout.fillWidth: true
                    studio: appPalette
                    text: qsTr("Confirm before closing a tab")
                    checked: appSettings.confirmCloseTab
                    onToggled: appSettings.setConfirmCloseTab(checked)
                }
                StudioCheck {
                    Layout.fillWidth: true
                    studio: appPalette
                    text: qsTr("Show welcome screen on startup")
                    checked: appSettings.showWelcomeOnStartup
                    onToggled: appSettings.setShowWelcomeOnStartup(checked)
                }
                StudioCheck {
                    Layout.fillWidth: true
                    studio: appPalette
                    text: qsTr("Offer last project on welcome screen")
                    checked: appSettings.restoreLastProject
                    onToggled: appSettings.setRestoreLastProject(checked)
                }
                StudioCheck {
                    Layout.fillWidth: true
                    studio: appPalette
                    text: qsTr("Autosave project")
                    checked: appSettings.projectAutosave
                    onToggled: appSettings.setProjectAutosave(checked)
                }
                StudioSpin {
                    Layout.fillWidth: true
                    studio: appPalette
                    label: qsTr("Autosave interval (sec)")
                    from: 30
                    to: 3600
                    stepSize: 30
                    value: appSettings.projectAutosaveSeconds
                    onValueCommitted: appSettings.setProjectAutosaveSeconds(newValue)
                }
                StudioField { studio: appPalette; labelText: qsTr("Documents root (restart)") }
                RowLayout {
                    Layout.fillWidth: true
                    spacing: appPalette.spacingSm
                    StudioTextField {
                        Layout.fillWidth: true
                        studio: appPalette
                        readOnly: true
                        text: appSettings.documentsRoot.length > 0
                            ? appSettings.documentsRoot
                            : qsTr("Default: Documents/PixelStudio")
                    }
                    StudioButton {
                        studio: appPalette
                        compact: true
                        text: qsTr("Browse…")
                        onClicked: {
                            folderPickTarget = 0
                            customFolderDialog.currentFolder = appSettings.documentsRoot.length > 0
                                ? localFolderUrl(appSettings.documentsRoot, pixelStudioDocumentsPath)
                                : pixelStudioDocumentsPath
                            customFolderDialog.open()
                        }
                    }
                    StudioButton {
                        studio: appPalette
                        compact: true
                        text: qsTr("Clear")
                        enabled: appSettings.documentsRoot.length > 0
                        onClicked: appSettings.setDocumentsRoot("")
                    }
                }
                StudioField { studio: appPalette; labelText: qsTr("Projects folder (restart)") }
                RowLayout {
                    Layout.fillWidth: true
                    spacing: appPalette.spacingSm
                    StudioTextField {
                        Layout.fillWidth: true
                        studio: appPalette
                        readOnly: true
                        text: appSettings.projectsRoot.length > 0
                            ? appSettings.projectsRoot
                            : qsTr("Empty = default")
                    }
                    StudioButton {
                        studio: appPalette
                        compact: true
                        text: qsTr("Browse…")
                        onClicked: {
                            folderPickTarget = 1
                            customFolderDialog.currentFolder = appSettings.projectsRoot.length > 0
                                ? localFolderUrl(appSettings.projectsRoot, pixelStudioProjectsUrl)
                                : pixelStudioProjectsUrl
                            customFolderDialog.open()
                        }
                    }
                    StudioButton {
                        studio: appPalette
                        compact: true
                        text: qsTr("Clear")
                        enabled: appSettings.projectsRoot.length > 0
                        onClicked: appSettings.setProjectsRoot("")
                    }
                }
                StudioField { studio: appPalette; labelText: qsTr("Exports folder (restart)") }
                RowLayout {
                    Layout.fillWidth: true
                    spacing: appPalette.spacingSm
                    StudioTextField {
                        Layout.fillWidth: true
                        studio: appPalette
                        readOnly: true
                        text: appSettings.exportsRoot.length > 0
                            ? appSettings.exportsRoot
                            : qsTr("Empty = default")
                    }
                    StudioButton {
                        studio: appPalette
                        compact: true
                        text: qsTr("Browse…")
                        onClicked: {
                            folderPickTarget = 2
                            customFolderDialog.currentFolder = appSettings.exportsRoot.length > 0
                                ? localFolderUrl(appSettings.exportsRoot, pixelStudioExportsUrl)
                                : pixelStudioExportsUrl
                            customFolderDialog.open()
                        }
                    }
                    StudioButton {
                        studio: appPalette
                        compact: true
                        text: qsTr("Clear")
                        enabled: appSettings.exportsRoot.length > 0
                        onClicked: appSettings.setExportsRoot("")
                    }
                }
                StudioButton {
                    Layout.fillWidth: true
                    studio: appPalette
                    text: qsTr("Reset UI defaults")
                    onClicked: appSettings.resetUiDefaults()
                }
            }
        }
    }

    Dialog {
        id: resetSessionConfirmDialog
        title: qsTr("Reset session?")
        modal: true
        anchors.centerIn: parent
        standardButtons: Dialog.Yes | Dialog.No
        padding: appPalette.spacingLg
        property int bodyWidth: window.dialogBodyWidth(320, appPalette.spacingXl * 4)
        width: bodyWidth + 2 * padding
        onAccepted: {
            converter.resetSession()
            statusMessage(qsTr("Session reset"))
        }
        background: Rectangle {
            radius: appPalette.radiusMd
            color: appPalette.surface
            border.width: 2
            border.color: appPalette.accent
        }
        contentItem: Label {
            width: resetSessionConfirmDialog.bodyWidth
            wrapMode: Text.WordWrap
            text: qsTr("Clears inspector settings, recent lists, and watch folders. Projects on disk are not deleted.")
            font.family: appPalette.fontFamily
            color: appPalette.text
        }
    }

    Dialog {
        id: closeTabConfirmDialog
        title: qsTr("Close tab?")
        modal: true
        anchors.centerIn: parent
        standardButtons: Dialog.Yes | Dialog.No
        padding: appPalette.spacingLg
        property int bodyWidth: window.dialogBodyWidth(360, appPalette.spacingXl * 4)
        width: bodyWidth + 2 * padding
        onAccepted: {
            if (closeTabDontAsk.checked)
                appSettings.setConfirmCloseTab(false)
            if (pendingCloseTabId.length > 0)
                tabController.closeTab(pendingCloseTabId)
            pendingCloseTabId = ""
        }
        onRejected: pendingCloseTabId = ""
        background: Rectangle {
            radius: appPalette.radiusMd
            color: appPalette.surface
            border.width: 2
            border.color: appPalette.accent
        }
        contentItem: ColumnLayout {
            width: closeTabConfirmDialog.bodyWidth
            spacing: appPalette.spacingMd
            Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                text: qsTr("Close “%1”? Unsaved changes in this tab may be lost.")
                    .arg(tabTitleForId(pendingCloseTabId))
                font.family: appPalette.fontFamily
                color: appPalette.text
            }
            StudioCheck {
                id: closeTabDontAsk
                studio: appPalette
                text: qsTr("Don't ask again")
            }
        }
    }

    Dialog {
        id: exitConfirmDialog
        title: qsTr("Exit PixelStudio?")
        modal: true
        anchors.centerIn: parent
        standardButtons: Dialog.Yes | Dialog.No
        padding: appPalette.spacingLg
        property int bodyWidth: window.dialogBodyWidth(360, appPalette.spacingXl * 4)
        width: bodyWidth + 2 * padding
        onAccepted: {
            forceClose = true
            window.close()
        }
        background: Rectangle {
            radius: appPalette.radiusMd
            color: appPalette.surface
            border.width: 2
            border.color: appPalette.accent
        }
        contentItem: Label {
            width: exitConfirmDialog.bodyWidth
            wrapMode: Text.WordWrap
            text: qsTr("Close the application? Unsaved exported files or project changes may be lost.")
            font.family: appPalette.fontFamily
            color: appPalette.text
        }
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
        onOpenImageTabRequested: openDialog.open()
        onSettingsRequested: preferencesDialog.open()
        onNewProjectRequested: tabController.newProjectTab(qsTr("Untitled"))
        onOpenRequested: openDialog.open()
        onSaveRequested: exportBridge.saveProject()
        onToggleGridRequested: appSettings.setShowPixelGrid(!appSettings.showPixelGrid)
        onViewDualRequested: setWorkspaceView(0)
        onViewSourceRequested: setWorkspaceView(1)
        onViewOutputRequested: setWorkspaceView(3)

        WelcomeScreen {
            anchors.fill: parent
            visible: tabController.activeIsWelcome
            studio: appPalette
            onOpenImageRequested: openDialog.open()
            onPasteRequested: importClipboard()
            onOpenProjectRequested: openProjectDialog.open()
            onNewProjectRequested: tabController.newProjectTab(qsTr("Untitled"))
            onContinueLastProjectRequested: {
                if (converter.hasRestorableProject)
                    tabController.openProjectTab(localFolderUrl(converter.lastProjectPath, pixelStudioProjectsUrl))
            }
            onRecentFileRequested: (path) => openLocalPath(path)
            onFileDropped: (urls) => openDroppedUrls(urls)
            onSettingsRequested: preferencesDialog.open()
        }

        StudioLayout {
            id: studioLayout
            anchors.fill: parent
            visible: !tabController.activeIsWelcome
            studio: appPalette
            viewMode: tabController.activeViewMode
            exportBridge: exportBridge
            onOpenRequested: function() { openDialog.open() }
            onPasteRequested: importClipboard()
            onViewModeRequested: (mode) => setWorkspaceView(mode)
            onAssetOpenRequested: (path) => openLocalPath(path)
            onSaveCodeRequested: saveCodeDialog.open()
            onSaveBinRequested: saveBinDialog.open()
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
