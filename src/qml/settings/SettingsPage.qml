import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import PixelStudio

pragma Translator: Shell


Item {
    id: root

    required property var studio
    required property var win
    required property var hostDialogs

    property int section: 0

    onVisibleChanged: {
        if (visible)
            section = WorkflowRouter.settingsSection
    }

    readonly property var sections: [
        { sectionId: 0, icon: "sliders-horizontal", label: qsTr("General") },
        { sectionId: 1, icon: "book", label: qsTr("Editor") },
        { sectionId: 2, icon: "house", label: qsTr("Startup") },
        { sectionId: 3, icon: "save", label: qsTr("Projects") },
        { sectionId: 4, icon: "folder-open", label: qsTr("Storage") },
        { sectionId: 5, icon: "download", label: qsTr("Data") }
    ]

    component SettingsCard: Rectangle {
        id: card
        required property var studio
        default property alias body: cardBody.data

        Layout.fillWidth: true
        implicitWidth: 480
        width: parent && parent.width > 0 ? parent.width : implicitWidth
        implicitHeight: cardBody.implicitHeight + studio.spacingLg * 2
        radius: studio.radiusMd
        color: studio.surface
        border.width: 1
        border.color: studio.border

        ColumnLayout {
            id: cardBody
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.margins: studio.spacingLg
            spacing: studio.spacingMd
        }
    }

    component ToggleRow: Item {
        id: toggleRow
        required property var studio
        property string title: ""
        property string description: ""
        property bool checked: false
        signal toggled(bool value)

        Layout.fillWidth: true
        implicitHeight: Math.max(toggleColumn.implicitHeight, toggleCheck.implicitHeight)

        ColumnLayout {
            id: toggleColumn
            anchors.left: parent.left
            anchors.right: toggleCheck.left
            anchors.rightMargin: studio.spacingMd
            anchors.verticalCenter: parent.verticalCenter
            spacing: 2

            Label {
                Layout.fillWidth: true
                text: toggleRow.title
                font.family: studio.fontFamily
                font.pixelSize: studio.fontSizeBase
                color: studio.text
                wrapMode: Text.WordWrap
            }
            Label {
                Layout.fillWidth: true
                visible: toggleRow.description.length > 0
                text: toggleRow.description
                font.family: studio.fontFamily
                font.pixelSize: studio.fontSizeSm
                color: studio.textMuted
                wrapMode: Text.WordWrap
            }
        }

        StudioCheck {
            id: toggleCheck
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            studio: toggleRow.studio
            text: ""
            checked: toggleRow.checked
            onToggled: function(checked) { toggleRow.toggled(checked) }
        }
    }

    component PathRow: ColumnLayout {
        id: pathRow
        required property var studio
        required property var win
        required property var hostDialogs
        property string title: ""
        property string description: ""
        property string path: ""
        property string emptyHint: ""
        property int pickTarget: 0
        property url fallbackUrl
        signal clearRequested()

        Layout.fillWidth: true
        spacing: studio.spacingSm

        Label {
            Layout.fillWidth: true
            text: pathRow.title
            font.family: studio.fontFamily
            font.pixelSize: studio.fontSizeBase
            color: studio.text
        }
        Label {
            Layout.fillWidth: true
            visible: pathRow.description.length > 0
            text: pathRow.description
            font.family: studio.fontFamily
            font.pixelSize: studio.fontSizeSm
            color: studio.textMuted
            wrapMode: Text.WordWrap
        }
        RowLayout {
            Layout.fillWidth: true
            spacing: studio.spacingSm
            StudioTextField {
                Layout.fillWidth: true
                studio: pathRow.studio
                readOnly: true
                text: pathRow.path.length > 0 ? pathRow.path : pathRow.emptyHint
            }
            StudioButton {
                studio: pathRow.studio
                compact: true
                text: qsTr("Browse…")
                onClicked: {
                    win.folderPickTarget = pathRow.pickTarget
                    hostDialogs.customFolderDialog.currentFolder = WorkflowRouter.localFolderUrl(
                        pathRow.path, pathRow.fallbackUrl)
                    hostDialogs.customFolderDialog.open()
                }
            }
            StudioButton {
                studio: pathRow.studio
                compact: true
                text: qsTr("Clear")
                enabled: pathRow.path.length > 0
                onClicked: pathRow.clearRequested()
            }
        }
    }

    StudioBackdrop {
        anchors.fill: parent
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: studio.spacing2xl
        spacing: studio.spacing2xl

        Rectangle {
            Layout.preferredWidth: 220
            Layout.fillHeight: true
            radius: studio.radiusMd
            color: studio.surfaceInset
            border.width: 1
            border.color: studio.border
            clip: true

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: studio.spacingMd
                spacing: studio.spacingXs

                Label {
                    Layout.fillWidth: true
                    Layout.bottomMargin: studio.spacingSm
                    text: qsTr("SETTINGS")
                    font.family: studio.fontFamilyPixel
                    font.pixelSize: studio.fontSizePixel
                    color: studio.accent
                }

                Repeater {
                    model: root.sections
                    delegate: SettingsNavButton {
                        required property var modelData
                        Layout.fillWidth: true
                        studio: root.studio
                        iconName: modelData.icon
                        label: modelData.label
                        active: root.section === modelData.sectionId
                        onTriggered: root.section = modelData.sectionId
                    }
                }

                Item { Layout.fillHeight: true }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: studio.radiusMd
            color: Qt.rgba(studio.surface.r, studio.surface.g, studio.surface.b, 0.72)
            border.width: 1
            border.color: studio.border
            clip: true

            ScrollView {
                id: settingsScroll
                anchors.fill: parent
                anchors.margins: studio.spacingLg
                clip: true
                ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

                ColumnLayout {
                    id: contentColumn
                    width: settingsScroll.availableWidth
                    spacing: studio.spacingLg

                    Label {
                        Layout.fillWidth: true
                        text: root.sections[root.section].label
                        font.family: studio.fontFamily
                        font.pixelSize: studio.fontSize2xl
                        font.weight: Font.DemiBold
                        color: studio.text
                    }

                    Label {
                        Layout.fillWidth: true
                        Layout.bottomMargin: studio.spacingSm
                        text: {
                            switch (root.section) {
                            case 0: return qsTr("Language, layout, and shell behavior.")
                            case 1: return qsTr("Code panel and syntax highlighting.")
                            case 2: return qsTr("Welcome screen and confirmations.")
                            case 3: return qsTr("Autosave and project workflow.")
                            case 4: return qsTr("Custom folders for documents, projects, and exports.")
                            case 5: return qsTr("Backup, restore, and maintenance.")
                            default: return ""
                            }
                        }
                        font.family: studio.fontFamily
                        font.pixelSize: studio.fontSizeSm
                        color: studio.textSecondary
                        wrapMode: Text.WordWrap
                    }

                    StackLayout {
                        id: sectionStack
                        Layout.fillWidth: true
                        Layout.preferredWidth: contentColumn.width
                        currentIndex: root.section

                    // General
                    SettingsCard {
                        Layout.fillWidth: true
                        Layout.preferredWidth: sectionStack.width
                        studio: root.studio

                        StudioField { studio: root.studio; labelText: qsTr("Language") }
                        StudioCombo {
                            id: languageCombo
                            Layout.fillWidth: true
                            studio: root.studio
                            model: workspace.settings.availableLanguages()
                            textRole: "name"
                            onActivated: {
                                const item = model[currentIndex]
                                if (item && item.code)
                                    workspace.settings.setLanguageCode(item.code)
                            }
                        }

                        Item {
                            id: languageCredits
                            Layout.fillWidth: true
                            implicitHeight: languageCreditsColumn.implicitHeight
                            visible: showCredits

                            readonly property var selectedLanguage: {
                                const langs = workspace.settings.availableLanguages()
                                const idx = languageCombo.currentIndex
                                if (idx < 0 || idx >= langs.length)
                                    return null
                                return langs[idx]
                            }
                            readonly property string authorName: {
                                const lang = selectedLanguage
                                return lang && lang.author ? String(lang.author) : ""
                            }
                            readonly property var contributorNames: {
                                const lang = selectedLanguage
                                return lang && lang.contributors ? lang.contributors : []
                            }
                            readonly property bool showCredits: selectedLanguage
                                && (authorName.length > 0 || contributorNames.length > 0)

                            ColumnLayout {
                                id: languageCreditsColumn
                                width: parent.width
                                spacing: studio.spacingXs

                                Text {
                                    Layout.fillWidth: true
                                    visible: languageCredits.authorName.length > 0
                                    wrapMode: Text.WordWrap
                                    font.family: studio.fontFamily
                                    font.pixelSize: studio.fontSizeXs
                                    color: studio.textMuted
                                    text: qsTr("Author: %1").arg(languageCredits.authorName)
                                }
                                Text {
                                    Layout.fillWidth: true
                                    visible: languageCredits.contributorNames.length > 0
                                    wrapMode: Text.WordWrap
                                    font.family: studio.fontFamily
                                    font.pixelSize: studio.fontSizeXs
                                    color: studio.textMuted
                                    text: qsTr("Contributors: %1").arg(
                                              languageCredits.contributorNames.join(", "))
                                }
                            }
                        }

                        ToggleRow {
                            studio: root.studio
                            title: qsTr("Show inspector panel")
                            description: qsTr("Right-side image and export controls.")
                            checked: workspace.settings.showSidebar
                            onToggled: function(value) { workspace.settings.setShowSidebar(value) }
                        }
                        ToggleRow {
                            studio: root.studio
                            title: qsTr("Show pixel grid")
                            description: qsTr("Overlay grid on the canvas preview.")
                            checked: workspace.viewport.showGrid
                            onToggled: function(value) {
                                workspace.viewport.setShowGrid(value)
                                workspace.settings.setShowPixelGrid(value)
                            }
                        }
                    }

                    // Editor
                    SettingsCard {
                        Layout.fillWidth: true
                        Layout.preferredWidth: sectionStack.width
                        studio: root.studio

                        ToggleRow {
                            studio: root.studio
                            title: qsTr("Wrap generated code")
                            description: qsTr("Soft-wrap long lines in the code panel.")
                            checked: workspace.settings.codeWrap
                            onToggled: function(value) { workspace.settings.setCodeWrap(value) }
                        }

                        StudioField {
                            studio: root.studio
                            labelText: qsTr("Code syntax colors")
                        }

                        SyntaxColorsEditor {
                            studio: root.studio
                        }
                    }

                    // Startup
                    SettingsCard {
                        Layout.fillWidth: true
                        Layout.preferredWidth: sectionStack.width
                        studio: root.studio

                        ToggleRow {
                            studio: root.studio
                            title: qsTr("Show welcome screen on startup")
                            checked: workspace.settings.showWelcomeOnStartup
                            onToggled: function(value) { workspace.settings.setShowWelcomeOnStartup(value) }
                        }
                        ToggleRow {
                            studio: root.studio
                            title: qsTr("Offer last project on welcome screen")
                            checked: workspace.settings.restoreLastProject
                            onToggled: function(value) { workspace.settings.setRestoreLastProject(value) }
                        }
                        ToggleRow {
                            studio: root.studio
                            title: qsTr("Confirm before exit")
                            checked: workspace.settings.confirmExit
                            onToggled: function(value) { workspace.settings.setConfirmExit(value) }
                        }
                        ToggleRow {
                            studio: root.studio
                            title: qsTr("Confirm before closing a tab")
                            checked: workspace.settings.confirmCloseTab
                            onToggled: function(value) { workspace.settings.setConfirmCloseTab(value) }
                        }
                    }

                    // Projects
                    SettingsCard {
                        Layout.fillWidth: true
                        Layout.preferredWidth: sectionStack.width
                        studio: root.studio

                        ToggleRow {
                            studio: root.studio
                            title: qsTr("Autosave project")
                            description: qsTr("Periodically save the active project to disk.")
                            checked: workspace.settings.projectAutosave
                            onToggled: function(value) { workspace.settings.setProjectAutosave(value) }
                        }
                        StudioSpin {
                            Layout.fillWidth: true
                            studio: root.studio
                            label: qsTr("Autosave interval (sec)")
                            from: 30
                            to: 3600
                            stepSize: 30
                            value: workspace.settings.projectAutosaveSeconds
                            enabled: workspace.settings.projectAutosave
                            onValueCommitted: function(newValue) {
                                workspace.settings.setProjectAutosaveSeconds(newValue)
                            }
                        }
                    }

                    // Storage
                    SettingsCard {
                        Layout.fillWidth: true
                        Layout.preferredWidth: sectionStack.width
                        studio: root.studio

                        PathRow {
                            studio: root.studio
                            win: root.win
                            hostDialogs: root.hostDialogs
                            title: qsTr("Documents root")
                            description: qsTr("Default location for user documents.")
                            path: workspace.settings.documentsRoot
                            emptyHint: qsTr("Default: Documents/PixelStudio")
                            pickTarget: 0
                            fallbackUrl: workspace.settings.documentsUrl
                            onClearRequested: workspace.settings.setDocumentsRoot("")
                        }
                        PathRow {
                            studio: root.studio
                            win: root.win
                            hostDialogs: root.hostDialogs
                            title: qsTr("Projects folder")
                            description: qsTr("Where .pixelstudio projects are stored.")
                            path: workspace.settings.projectsRoot
                            emptyHint: qsTr("Empty = default")
                            pickTarget: 1
                            fallbackUrl: workspace.settings.projectsUrl
                            onClearRequested: workspace.settings.setProjectsRoot("")
                        }
                        PathRow {
                            studio: root.studio
                            win: root.win
                            hostDialogs: root.hostDialogs
                            title: qsTr("Exports folder")
                            description: qsTr("Default batch export destination.")
                            path: workspace.settings.exportsRoot
                            emptyHint: qsTr("Empty = default")
                            pickTarget: 2
                            fallbackUrl: workspace.settings.exportsUrl
                            onClearRequested: workspace.settings.setExportsRoot("")
                        }
                    }

                    // Data
                    SettingsCard {
                        Layout.fillWidth: true
                        Layout.preferredWidth: sectionStack.width
                        studio: root.studio

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: studio.spacingSm
                            StudioButton {
                                Layout.fillWidth: true
                                studio: root.studio
                                text: qsTr("Export settings backup…")
                                onClicked: hostDialogs.settingsExportDialog.open()
                            }
                            StudioButton {
                                Layout.fillWidth: true
                                studio: root.studio
                                text: qsTr("Import settings backup…")
                                onClicked: hostDialogs.settingsImportDialog.open()
                            }
                        }
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: studio.spacingSm
                            StudioButton {
                                Layout.fillWidth: true
                                studio: root.studio
                                text: qsTr("Open app data folder")
                                onClicked: workspace.project.openAppDataFolder()
                            }
                            StudioButton {
                                Layout.fillWidth: true
                                studio: root.studio
                                text: qsTr("Open documents folder")
                                onClicked: workspace.project.openUserDocumentsFolder()
                            }
                        }
                        StudioButton {
                            Layout.fillWidth: true
                            studio: root.studio
                            text: qsTr("Open logs folder")
                            onClicked: workspace.project.openLogsFolder()
                        }
                        StudioButton {
                            Layout.fillWidth: true
                            studio: root.studio
                            text: qsTr("Reset UI defaults")
                            onClicked: workspace.settings.resetUiDefaults()
                        }
                        StudioButton {
                            Layout.fillWidth: true
                            studio: root.studio
                            text: qsTr("Reset session…")
                            onClicked: hostDialogs.resetSessionConfirmDialog.open()
                        }
                    }

                    } // StackLayout

                    Item { Layout.fillHeight: true; Layout.minimumHeight: studio.spacingLg }
                }
            }
        }
    }

    Connections {
        target: workspace.settings
        function onLanguageCodeChanged() {
            languageCombo.currentIndex = WorkflowRouter.languageIndex()
        }
    }

    Component.onCompleted: languageCombo.currentIndex = WorkflowRouter.languageIndex()
}
