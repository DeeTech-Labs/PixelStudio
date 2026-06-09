pragma Singleton

import QtQuick

pragma Translator: Shell


QtObject {
    id: root

    readonly property var toolbarItems: [
        { icon: "plus", tip: "New", command: "newProject", enabled: true },
        { icon: "folder-open", tip: "Open", command: "openImage", enabled: true },
        { icon: "save", tip: "Save", command: "saveProject", enabled: true },
        { sep: true },
        { icon: "layout-grid", tip: "Toggle grid", command: "toggleGrid", enabled: true, toggled: true },
        { icon: "columns-2", tip: "Dual view", command: "viewDual", enabled: true },
        { icon: "image", tip: "Source", command: "viewSource", enabled: true },
        { icon: "file-image", tip: "Output", command: "viewOutput", enabled: true }
    ]

    readonly property var menuSections: ({
        file: [
            { label: "Welcome screen", command: "welcome" },
            { separator: true },
            { label: "New project", command: "newProject", shortcut: "Ctrl+N" },
            { label: "Open project…", command: "openProject", shortcut: "Ctrl+Shift+O" },
            { label: "Save project", command: "saveProject", shortcut: "Ctrl+Shift+S" },
            { label: "Save project as…", command: "saveProjectAs" },
            { separator: true },
            { label: "Open image…", command: "openImage", shortcut: "Ctrl+O" },
            { label: "Import C header…", command: "importHeader" },
            { label: "Paste from clipboard", command: "importClipboard", shortcut: "Ctrl+V" },
            { separator: true },
            { label: "Import settings backup…", command: "importSettings" },
            { separator: true },
            { label: "Clear project", command: "clearProject" },
            { separator: true },
            { label: "Exit", command: "exit", shortcut: "Ctrl+Q" }
        ],
        edit: [
            { label: "Copy generated code", command: "copyCode", shortcut: "Ctrl+C" },
            { separator: true },
            { label: "Clear image", command: "clearImage" }
        ],
        view: [
            { label: "Inspector panel", command: "toggleSidebar", shortcut: "Ctrl+B", checkable: true },
            { separator: true },
            { label: "View: Dual", command: "viewDual" },
            { label: "View: Source", command: "viewSource" },
            { label: "View: Output", command: "viewOutput" },
            { separator: true },
            { label: "Wrap generated code", command: "toggleCodeWrap", checkable: true },
            { label: "Show pixel grid", command: "toggleGrid", checkable: true }
        ],
        image: [
            { label: "Rotate 90° clockwise", command: "rotateCw", shortcut: "Ctrl+R" },
            { label: "Flip horizontally", command: "flipH", checkable: true },
            { label: "Flip vertically", command: "flipV", checkable: true },
            { label: "Invert result colors", command: "invertColors", checkable: true },
            { separator: true },
            { label: "Swap display width and height", command: "swapDisplay" }
        ],
        tools: [
            { label: "Batch export .h…", command: "batchExport", shortcut: "Ctrl+Shift+B" },
            { label: "Build sprite atlas…", command: "buildAtlas" },
            { label: "Import C header…", command: "importHeader" },
            { separator: true },
            { label: "Configure watch folder…", command: "watchFolder" },
            { label: "Watch folder", command: "toggleWatch", checkable: true }
        ],
        export: [
            { label: "Export panel…", command: "exportHub", shortcut: "Ctrl+Shift+E" },
            { separator: true },
            { label: "Save code to file…", command: "saveCode", shortcut: "Ctrl+Alt+S" },
            { label: "Save binary…", command: "saveBin" },
            { separator: true },
            { label: "Batch export .h…", command: "batchExport", shortcut: "Ctrl+Shift+B" },
            { label: "Build sprite atlas…", command: "buildAtlas" },
            { separator: true },
            { label: "Export settings backup…", command: "exportSettings" }
        ],
        help: [
            { label: "Preferences…", command: "preferences", shortcut: "Ctrl+," },
            { separator: true },
            { label: "Reset UI defaults", command: "resetUi" },
            { label: "Open settings folder", command: "openSettingsFolder" },
            { label: "Import settings backup…", command: "importSettings" },
            { label: "Export settings backup…", command: "exportSettings" },
            { label: "Reset session", command: "resetSession" },
            { separator: true },
            { label: "Open PixelStudio folder", command: "openDocumentsFolder" },
            { separator: true },
            { label: "About PixelStudio", command: "about" }
        ]
    })
}
