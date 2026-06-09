import QtQuick
import QtQuick.Controls
import PixelStudio

pragma Translator: Shell


MenuBar {
    id: root
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
            text: WorkflowRouter.stripMenuMnemonic(menuBarItem.text)
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
        title: WorkflowRouter.shellTr("&File")

        Action {
            text: WorkflowRouter.shellTr("Welcome screen")
            onTriggered: WorkflowRouter.run("welcome")
        }
        MenuSeparator {}
        Action {
            text: WorkflowRouter.shellTr("New project")
            shortcut: "Ctrl+N"
            onTriggered: WorkflowRouter.run("newProject")
        }
        Action {
            text: WorkflowRouter.shellTr("Open project…")
            shortcut: "Ctrl+Shift+O"
            onTriggered: WorkflowRouter.run("openProject")
        }
        Action {
            text: WorkflowRouter.shellTr("Save project")
            shortcut: "Ctrl+Shift+S"
            onTriggered: WorkflowRouter.run("saveProject")
        }
        Action {
            text: WorkflowRouter.shellTr("Save project as…")
            onTriggered: WorkflowRouter.run("saveProjectAs")
        }
        MenuSeparator {}
        Menu {
            id: menuBarImportMenu
            title: WorkflowRouter.shellTr("Import")

            Action {
                text: WorkflowRouter.shellTr("Open image…")
                shortcut: "Ctrl+O"
                onTriggered: WorkflowRouter.run("openImage")
            }
            Action {
                text: WorkflowRouter.shellTr("Import C header…")
                onTriggered: WorkflowRouter.run("importHeader")
            }
            Action {
                text: WorkflowRouter.shellTr("Paste from clipboard")
                shortcut: "Ctrl+V"
                onTriggered: WorkflowRouter.run("importClipboard")
            }
            MenuSeparator {}
            Menu {
                id: openRecentMenu
                title: WorkflowRouter.shellTr("Open recent")
                enabled: workspace.project.recentFiles.length > 0

                Instantiator {
                    model: workspace.project.recentFiles
                    delegate: MenuItem {
                        required property var modelData
                        text: modelData.name
                        onTriggered: WorkflowRouter.openLocalPath(modelData.path)
                    }
                    onObjectAdded: (index, object) => openRecentMenu.insertItem(index, object)
                    onObjectRemoved: (index, object) => openRecentMenu.removeItem(object)
                }
            }
            MenuSeparator {}
            Action {
                text: WorkflowRouter.shellTr("Import settings backup…")
                onTriggered: WorkflowRouter.run("importSettings")
            }
        }
        Menu {
            id: menuBarExportMenu
            title: WorkflowRouter.shellTr("Export")

            Action {
                text: WorkflowRouter.shellTr("Export panel…")
                shortcut: "Ctrl+Shift+E"
                onTriggered: WorkflowRouter.run("exportHub")
            }
            MenuSeparator {}
            Action {
                text: WorkflowRouter.shellTr("Save code to file…")
                shortcut: "Ctrl+Alt+S"
                enabled: WorkflowRouter.commandEnabled("saveCode")
                onTriggered: WorkflowRouter.run("saveCode")
            }
            Action {
                text: WorkflowRouter.shellTr("Save binary…")
                enabled: WorkflowRouter.commandEnabled("saveBin")
                onTriggered: WorkflowRouter.run("saveBin")
            }
            MenuSeparator {}
            Action {
                text: WorkflowRouter.shellTr("Batch export .h…")
                shortcut: "Ctrl+Shift+B"
                onTriggered: WorkflowRouter.run("batchExport")
            }
            Action {
                text: WorkflowRouter.shellTr("Build sprite atlas…")
                onTriggered: WorkflowRouter.run("buildAtlas")
            }
            MenuSeparator {}
            Menu {
                id: menuBarRecentExportsMenu
                title: WorkflowRouter.shellTr("Recent exports")
                enabled: workspace.exportPanel.recentExports.length > 0

                Instantiator {
                    model: workspace.exportPanel.recentExports
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
                text: WorkflowRouter.shellTr("Export settings backup…")
                onTriggered: WorkflowRouter.run("exportSettings")
            }
        }
        MenuSeparator {}
        Action {
            text: WorkflowRouter.shellTr("&Clear project")
            enabled: WorkflowRouter.commandEnabled("clearProject")
            onTriggered: WorkflowRouter.run("clearProject")
        }
        MenuSeparator {}
        Action {
            text: WorkflowRouter.shellTr("E&xit")
            shortcut: "Ctrl+Q"
            onTriggered: WorkflowRouter.run("exit")
        }
    }

    StudioMenuFromCommands {
        title: WorkflowRouter.shellTr("&Edit")
        commands: StudioCommands.menuSections.edit
    }

    StudioMenuFromCommands {
        title: WorkflowRouter.shellTr("&View")
        commands: StudioCommands.menuSections.view
    }

    StudioMenuFromCommands {
        title: WorkflowRouter.shellTr("&Image")
        commands: StudioCommands.menuSections.image
        menuEnabled: workspace.image.hasImage
    }

    StudioMenuFromCommands {
        title: WorkflowRouter.shellTr("&Tools")
        commands: StudioCommands.menuSections.tools
    }

    Menu {
        title: WorkflowRouter.shellTr("&Help")

        Action {
            text: WorkflowRouter.shellTr("Preferences…")
            shortcut: "Ctrl+,"
            onTriggered: WorkflowRouter.run("preferences")
        }
        Menu {
            id: languageMenu
            title: WorkflowRouter.shellTr("Language")

            Instantiator {
                model: workspace.settings.availableLanguages()
                delegate: MenuItem {
                    required property var modelData
                    text: modelData.name
                    checkable: true
                    checked: workspace.settings.languageCode === modelData.code
                    onTriggered: workspace.settings.setLanguageCode(modelData.code)
                }
                onObjectAdded: (index, object) => languageMenu.insertItem(index, object)
                onObjectRemoved: (index, object) => languageMenu.removeItem(object)
            }
        }
        MenuSeparator {}
        Action {
            text: WorkflowRouter.shellTr("Reset UI defaults")
            onTriggered: WorkflowRouter.run("resetUi")
        }
        Action {
            text: WorkflowRouter.shellTr("Open settings folder")
            onTriggered: WorkflowRouter.run("openSettingsFolder")
        }
        Action {
            text: WorkflowRouter.shellTr("Import settings backup…")
            onTriggered: WorkflowRouter.run("importSettings")
        }
        Action {
            text: WorkflowRouter.shellTr("Export settings backup…")
            onTriggered: WorkflowRouter.run("exportSettings")
        }
        Action {
            text: WorkflowRouter.shellTr("Reset session")
            onTriggered: WorkflowRouter.run("resetSession")
        }
        MenuSeparator {}
        Action {
            text: WorkflowRouter.shellTr("Open PixelStudio folder")
            onTriggered: WorkflowRouter.run("openDocumentsFolder")
        }
        MenuSeparator {}
        Action {
            text: WorkflowRouter.shellTr("About PixelStudio")
            onTriggered: WorkflowRouter.run("about")
        }
    }
}
