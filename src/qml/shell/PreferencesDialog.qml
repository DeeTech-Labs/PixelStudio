import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import PixelStudio

pragma Translator: Shell


Dialog {
    id: root
    required property var win
    required property var theme
    required property var hostDialogs

    modal: true
    standardButtons: Dialog.NoButton
    padding: theme.spacingLg
    property int bodyWidth: win.dialogBodyWidth(488, theme.spacingXl * 4)
    width: bodyWidth + 2 * padding
    title: qsTr("Preferences")

    background: Rectangle {
        radius: theme.radiusMd
        color: theme.surface
        border.width: 2
        border.color: theme.accent
    }

    ColumnLayout {
        width: root.bodyWidth
        spacing: theme.spacingMd

            StudioSection {
                Layout.fillWidth: true
                studio: theme
                title: qsTr("Application")
                hint: qsTr("Shell layout and language.")

                StudioField { studio: theme; labelText: qsTr("Language") }
                StudioCombo {
                    id: languageCombo
                    Layout.fillWidth: true
                    studio: theme
                    model: appSettings.availableLanguages()
                    textRole: "name"
                    currentIndex: win.languageIndex()
                    onActivated: {
                        const item = model[currentIndex]
                        if (item && item.code)
                            appSettings.setLanguageCode(item.code)
                    }
                }

                Item {
                    id: languageCredits
                    Layout.fillWidth: true
                    implicitHeight: creditsColumn.implicitHeight

                    readonly property var selectedLanguage: {
                        const langs = appSettings.availableLanguages()
                        const idx = languageCombo.currentIndex
                        if (idx < 0 || idx >= langs.length)
                            return null
                        return langs[idx]
                    }

                    ColumnLayout {
                        id: creditsColumn
                        width: parent.width
                        spacing: theme.spacingXs
                        visible: languageCredits.selectedLanguage
                                 && languageCredits.selectedLanguage.code !== "system"
                                 && (languageCredits.selectedLanguage.author
                                     || (languageCredits.selectedLanguage.contributors
                                         && languageCredits.selectedLanguage.contributors.length > 0))

                        Text {
                            Layout.fillWidth: true
                            visible: languageCredits.selectedLanguage
                                     && languageCredits.selectedLanguage.author
                            wrapMode: Text.WordWrap
                            font.family: theme.fontFamily
                            font.pixelSize: theme.fontSizeXs
                            color: theme.textMuted
                            text: languageCredits.selectedLanguage
                                  ? qsTr("Author: %1").arg(languageCredits.selectedLanguage.author)
                                  : ""
                        }
                        Text {
                            Layout.fillWidth: true
                            visible: languageCredits.selectedLanguage
                                     && languageCredits.selectedLanguage.contributors
                                     && languageCredits.selectedLanguage.contributors.length > 0
                            wrapMode: Text.WordWrap
                            font.family: theme.fontFamily
                            font.pixelSize: theme.fontSizeXs
                            color: theme.textMuted
                            text: languageCredits.selectedLanguage
                                  ? qsTr("Contributors: %1").arg(
                                        languageCredits.selectedLanguage.contributors.join(", "))
                                  : ""
                        }
                    }
                }

                StudioCheck {
                    Layout.fillWidth: true
                    studio: theme
                    text: qsTr("Show inspector panel")
                    checked: appSettings.showSidebar
                    onToggled: appSettings.setShowSidebar(checked)
                }
                StudioCheck {
                    Layout.fillWidth: true
                    studio: theme
                    text: qsTr("Show pixel grid")
                    checked: appSettings.showPixelGrid
                    onToggled: appSettings.setShowPixelGrid(checked)
                }
                StudioCheck {
                    Layout.fillWidth: true
                    studio: theme
                    text: qsTr("Wrap generated code")
                    checked: appSettings.codeWrap
                    onToggled: appSettings.setCodeWrap(checked)
                }
                StudioButton {
                    Layout.fillWidth: true
                    studio: theme
                    text: qsTr("Code syntax colors…")
                    onClicked: hostDialogs.syntaxColorsDialog.open()
                }
                StudioCheck {
                    Layout.fillWidth: true
                    studio: theme
                    text: qsTr("Confirm before exit")
                    checked: appSettings.confirmExit
                    onToggled: appSettings.setConfirmExit(checked)
                }
                StudioCheck {
                    Layout.fillWidth: true
                    studio: theme
                    text: qsTr("Confirm before closing a tab")
                    checked: appSettings.confirmCloseTab
                    onToggled: appSettings.setConfirmCloseTab(checked)
                }
                StudioCheck {
                    Layout.fillWidth: true
                    studio: theme
                    text: qsTr("Show welcome screen on startup")
                    checked: appSettings.showWelcomeOnStartup
                    onToggled: appSettings.setShowWelcomeOnStartup(checked)
                }
                StudioCheck {
                    Layout.fillWidth: true
                    studio: theme
                    text: qsTr("Offer last project on welcome screen")
                    checked: appSettings.restoreLastProject
                    onToggled: appSettings.setRestoreLastProject(checked)
                }
                StudioCheck {
                    Layout.fillWidth: true
                    studio: theme
                    text: qsTr("Autosave project")
                    checked: appSettings.projectAutosave
                    onToggled: appSettings.setProjectAutosave(checked)
                }
                StudioSpin {
                    Layout.fillWidth: true
                    studio: theme
                    label: qsTr("Autosave interval (sec)")
                    from: 30
                    to: 3600
                    stepSize: 30
                    value: appSettings.projectAutosaveSeconds
                    onValueCommitted: appSettings.setProjectAutosaveSeconds(newValue)
                }
                StudioField { studio: theme; labelText: qsTr("Documents root (restart)") }
                RowLayout {
                    Layout.fillWidth: true
                    spacing: theme.spacingSm
                    StudioTextField {
                        Layout.fillWidth: true
                        studio: theme
                        readOnly: true
                        text: appSettings.documentsRoot.length > 0
                            ? appSettings.documentsRoot
                            : qsTr("Default: Documents/PixelStudio")
                    }
                    StudioButton {
                        studio: theme
                        compact: true
                        text: qsTr("Browse…")
                        onClicked: {
                            win.folderPickTarget = 0
                            hostDialogs.customFolderDialog.currentFolder = appSettings.documentsRoot.length > 0
                                ? win.localFolderUrl(appSettings.documentsRoot, pixelStudioDocumentsPath)
                                : pixelStudioDocumentsPath
                            hostDialogs.customFolderDialog.open()
                        }
                    }
                    StudioButton {
                        studio: theme
                        compact: true
                        text: qsTr("Clear")
                        enabled: appSettings.documentsRoot.length > 0
                        onClicked: appSettings.setDocumentsRoot("")
                    }
                }
                StudioField { studio: theme; labelText: qsTr("Projects folder (restart)") }
                RowLayout {
                    Layout.fillWidth: true
                    spacing: theme.spacingSm
                    StudioTextField {
                        Layout.fillWidth: true
                        studio: theme
                        readOnly: true
                        text: appSettings.projectsRoot.length > 0
                            ? appSettings.projectsRoot
                            : qsTr("Empty = default")
                    }
                    StudioButton {
                        studio: theme
                        compact: true
                        text: qsTr("Browse…")
                        onClicked: {
                            win.folderPickTarget = 1
                            hostDialogs.customFolderDialog.currentFolder = appSettings.projectsRoot.length > 0
                                ? win.localFolderUrl(appSettings.projectsRoot, pixelStudioProjectsUrl)
                                : pixelStudioProjectsUrl
                            hostDialogs.customFolderDialog.open()
                        }
                    }
                    StudioButton {
                        studio: theme
                        compact: true
                        text: qsTr("Clear")
                        enabled: appSettings.projectsRoot.length > 0
                        onClicked: appSettings.setProjectsRoot("")
                    }
                }
                StudioField { studio: theme; labelText: qsTr("Exports folder (restart)") }
                RowLayout {
                    Layout.fillWidth: true
                    spacing: theme.spacingSm
                    StudioTextField {
                        Layout.fillWidth: true
                        studio: theme
                        readOnly: true
                        text: appSettings.exportsRoot.length > 0
                            ? appSettings.exportsRoot
                            : qsTr("Empty = default")
                    }
                    StudioButton {
                        studio: theme
                        compact: true
                        text: qsTr("Browse…")
                        onClicked: {
                            win.folderPickTarget = 2
                            hostDialogs.customFolderDialog.currentFolder = appSettings.exportsRoot.length > 0
                                ? win.localFolderUrl(appSettings.exportsRoot, pixelStudioExportsUrl)
                                : pixelStudioExportsUrl
                            hostDialogs.customFolderDialog.open()
                        }
                    }
                    StudioButton {
                        studio: theme
                        compact: true
                        text: qsTr("Clear")
                        enabled: appSettings.exportsRoot.length > 0
                        onClicked: appSettings.setExportsRoot("")
                    }
                }
                StudioButton {
                    Layout.fillWidth: true
                    studio: theme
                    text: qsTr("Reset UI defaults")
                    onClicked: appSettings.resetUiDefaults()
                }
            }
        }

    footer: StudioOkFooter {
        studio: theme
        dialog: root
    }
}