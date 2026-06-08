import QtQuick
import QtQuick.Controls
import PixelStudio

pragma Translator: PixelStudio

MenuBar {
    id: root
    required property var win
    required property var dialogs
    required property var studio

    spacing: 0

    background: Rectangle {
        color: studio.backgroundElevated
        implicitHeight: studio.menuBarHeight

        Rectangle {
            anchors.bottom: parent.bottom
            width: parent.width
            height: 1
            color: studio.border
        }
    }

    delegate: MenuBarItem {
        id: menuBarItem
        padding: studio.spacingMd

        contentItem: Label {
            text: win.stripMenuMnemonic(menuBarItem.text)
            font.family: studio.fontFamily
            font.pixelSize: studio.fontSizeSm
            color: menuBarItem.highlighted ? studio.text : studio.textSecondary
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        background: Rectangle {
            color: menuBarItem.highlighted ? studio.surfaceHover : "transparent"
        }
    }

    Menu {
        title: qsTr("&File")

        Action {
            text: qsTr("Welcome screen")
            onTriggered: win.openWelcome()
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
            onTriggered: dialogs.openProjectDialog.open()
        }
        Action {
            text: qsTr("Save project")
            shortcut: "Ctrl+Shift+S"
            onTriggered: {
                if (!converter.saveProject())
                    dialogs.saveProjectDialog.open()
            }
        }
        Action {
            text: qsTr("Save project as…")
            onTriggered: dialogs.saveProjectDialog.open()
        }
        MenuSeparator {}
        Menu {
            id: menuBarImportMenu
            title: qsTr("Import")
            Action {
                text: qsTr("Open image…")
                shortcut: "Ctrl+O"
                onTriggered: dialogs.openDialog.open()
            }
            Action {
                text: qsTr("Import C header…")
                onTriggered: dialogs.importHeaderDialog.open()
            }
            Action {
                text: qsTr("Paste from clipboard")
                shortcut: "Ctrl+V"
                onTriggered: win.importClipboard()
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
                        onTriggered: win.openLocalPath(modelData.path)
                    }
                    onObjectAdded: (index, object) => openRecentMenu.insertItem(index, object)
                    onObjectRemoved: (index, object) => openRecentMenu.removeItem(object)
                }
            }
            MenuSeparator {}
            Action {
                text: qsTr("Import settings backup…")
                onTriggered: dialogs.settingsImportDialog.open()
            }
        }
        Menu {
            id: menuBarExportMenu
            title: qsTr("Export")
            Action {
                text: qsTr("Export panel…")
                shortcut: "Ctrl+Shift+E"
                onTriggered: win.exportBridge.openExportHub()
            }
            MenuSeparator {}
            Action {
                text: qsTr("Save code to file…")
                shortcut: "Ctrl+Alt+S"
                enabled: converter.generatedCode.length > 0
                onTriggered: win.exportBridge.saveCode()
            }
            Action {
                text: qsTr("Save binary…")
                enabled: converter.hasPreview
                onTriggered: win.exportBridge.saveBin()
            }
            MenuSeparator {}
            Action {
                text: qsTr("Batch export .h…")
                shortcut: "Ctrl+Shift+B"
                onTriggered: win.exportBridge.batchExport()
            }
            Action {
                text: qsTr("Build sprite atlas…")
                onTriggered: win.exportBridge.buildAtlas()
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
                onTriggered: dialogs.settingsExportDialog.open()
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
            onTriggered: win.close()
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
                win.statusMessage(qsTr("Copied to clipboard"))
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
        MenuSeparator {}
        Action {
            text: qsTr("View: Dual")
            onTriggered: win.setWorkspaceView(0)
        }
        Action {
            text: qsTr("View: Source")
            onTriggered: win.setWorkspaceView(1)
        }
        Action {
            text: qsTr("View: Output")
            onTriggered: win.setWorkspaceView(3)
        }
        MenuSeparator {}
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
        title: qsTr("&Tools")
        Action {
            text: qsTr("Batch export .h…")
            shortcut: "Ctrl+Shift+B"
            onTriggered: dialogs.batchOpenDialog.open()
        }
        Action {
            text: qsTr("Build sprite atlas…")
            onTriggered: dialogs.atlasOpenDialog.open()
        }
        Action {
            text: qsTr("Import C header…")
            onTriggered: dialogs.importHeaderDialog.open()
        }
        MenuSeparator {}
        Action {
            text: qsTr("Configure watch folder…")
            onTriggered: dialogs.watchInputDialog.open()
        }
        Action {
            text: qsTr("Watch folder")
            checkable: true
            checked: converter.watchFolderActive
            onTriggered: converter.setWatchFolderActive(!converter.watchFolderActive)
        }
    }

    Menu {
        title: qsTr("&Help")
        Action {
            text: qsTr("Preferences…")
            shortcut: "Ctrl+,"
            onTriggered: dialogs.preferencesDialog.open()
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
        MenuSeparator {}
        Action {
            text: qsTr("Reset UI defaults")
            onTriggered: appSettings.resetUiDefaults()
        }
        Action {
            text: qsTr("Open settings folder")
            onTriggered: converter.openAppDataFolder()
        }
        Action {
            text: qsTr("Import settings backup…")
            onTriggered: dialogs.settingsImportDialog.open()
        }
        Action {
            text: qsTr("Export settings backup…")
            onTriggered: dialogs.settingsExportDialog.open()
        }
        Action {
            text: qsTr("Reset session")
            onTriggered: dialogs.resetSessionConfirmDialog.open()
        }
        MenuSeparator {}
        Action {
            text: qsTr("Open PixelStudio folder")
            onTriggered: converter.openUserDocumentsFolder()
        }
        MenuSeparator {}
        Action {
            text: qsTr("About PixelStudio")
            onTriggered: dialogs.aboutDialog.open()
        }
    }
}
