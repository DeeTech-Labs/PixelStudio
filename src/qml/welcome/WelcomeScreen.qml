import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import PixelStudio

pragma Translator: PixelStudio

DropArea {
    id: root
    required property var studio

    signal openImageRequested()
    signal pasteRequested()
    signal openProjectRequested()
    signal newProjectRequested()
    signal continueLastProjectRequested()
    signal recentFileRequested(string path)
    signal fileDropped(var urls)

    onDropped: (drop) => {
        if (drop.hasUrls && drop.urls.length > 0)
            root.fileDropped(drop.urls)
    }

    WelcomeBackdrop {
        anchors.fill: parent
        studio: root.studio
    }

    Rectangle {
        anchors.fill: parent
        visible: root.containsDrag
        color: Qt.rgba(59/255, 158/255, 1, 0.08)
        border.width: 2
        border.color: studio.accent
        opacity: 0.5
        radius: 0
    }

    Item {
        id: content
        anchors.fill: parent
        opacity: 0

        Component.onCompleted: contentEnter.start()
        NumberAnimation {
            id: contentEnter
            target: content
            property: "opacity"
            to: 1
            duration: 520
            easing.type: Easing.OutCubic
        }

        Item {
            width: Math.min(parent.width - 96, 1080)
            height: parent.height - 96
            anchors.centerIn: parent

        ColumnLayout {
            anchors.fill: parent
            spacing: 36

            RowLayout {
                Layout.fillWidth: true
                spacing: 48

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.maximumWidth: 520
                    spacing: studio.spacingLg

                    Label {
                        text: qsTr("PixelStudio")
                        font.pixelSize: 56
                        font.weight: Font.DemiBold
                        color: studio.text
                    }

                    Label {
                        Layout.fillWidth: true
                        wrapMode: Text.WordWrap
                        text: qsTr("Images to firmware-ready display assets")
                        font.pixelSize: 17
                        lineHeight: 1.35
                        color: studio.textSecondary
                    }

                    Rectangle {
                        Layout.preferredWidth: 64
                        Layout.preferredHeight: 3
                        radius: 2
                        color: studio.accent
                        opacity: 0.9
                    }

                    RowLayout {
                        spacing: studio.spacingMd
                        Rectangle {
                            implicitWidth: verChip.implicitWidth + studio.spacingLg * 2
                            implicitHeight: 30
                            radius: studio.radiusPill
                            color: Qt.rgba(59/255, 158/255, 1, 0.12)
                            border.width: 1
                            border.color: Qt.rgba(59/255, 158/255, 1, 0.35)
                            Label {
                                id: verChip
                                anchors.centerIn: parent
                                text: "v" + applicationVersion
                                font.family: studio.fontFamilyMono
                                font.pixelSize: studio.fontSizeSm
                                font.weight: Font.Medium
                                color: studio.accent
                            }
                        }
                        Label {
                            visible: root.containsDrag
                            text: qsTr("Release to open")
                            font.pixelSize: studio.fontSizeSm
                            color: studio.accent
                        }
                    }
                }

                Item {
                    Layout.preferredWidth: 300
                    Layout.preferredHeight: 140
                    Layout.alignment: Qt.AlignTop | Qt.AlignRight

                    WelcomeDisplayViz {
                        anchors.right: parent.right
                        anchors.top: parent.top
                        studio: root.studio
                    }
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: studio.spacingMd

                Label {
                    text: qsTr("Quick actions").toUpperCase()
                    font.pixelSize: 10
                    font.weight: Font.DemiBold
                    font.letterSpacing: 1.2
                    color: studio.textMuted
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: studio.spacingMd

                    StudioButton {
                        Layout.preferredWidth: 192
                        studio: root.studio
                        primary: true
                        iconName: "upload"
                        text: qsTr("Open image…")
                        onClicked: root.openImageRequested()
                    }
                    StudioButton {
                        Layout.preferredWidth: 192
                        studio: root.studio
                        iconName: "folder-open"
                        text: qsTr("Open project…")
                        onClicked: root.openProjectRequested()
                    }
                    StudioButton {
                        Layout.preferredWidth: 156
                        studio: root.studio
                        iconName: "clipboard"
                        text: qsTr("Paste")
                        onClicked: root.pasteRequested()
                    }
                    StudioButton {
                        Layout.preferredWidth: 156
                        studio: root.studio
                        iconName: "plus"
                        text: qsTr("New project")
                        onClicked: root.newProjectRequested()
                    }
                    StudioButton {
                        Layout.preferredWidth: 172
                        studio: root.studio
                        visible: appSettings.restoreLastProject && converter.hasRestorableProject
                        text: qsTr("Continue last")
                        onClicked: root.continueLastProjectRequested()
                    }
                    Item { Layout.fillWidth: true }
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: studio.spacingMd

                Label {
                    text: qsTr("Recent files").toUpperCase()
                    font.pixelSize: 10
                    font.weight: Font.DemiBold
                    font.letterSpacing: 1.2
                    color: studio.textMuted
                }

                Item {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 88
                    Layout.fillHeight: true

                    RowLayout {
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: studio.spacingLg
                        visible: converter.recentFiles.length === 0
                        opacity: visible ? 1 : 0

                        Rectangle {
                            width: 56
                            height: 56
                            radius: 28
                            color: studio.surface
                            border.width: 1
                            border.color: studio.border
                            StudioIcon {
                                anchors.centerIn: parent
                                name: "file-image"
                                iconSize: 24
                                tint: studio.textMuted
                            }
                        }
                        ColumnLayout {
                            spacing: studio.spacingSm
                            Label {
                                text: qsTr("No recent files yet")
                                font.pixelSize: studio.fontSizeLg
                                font.weight: Font.Medium
                                color: studio.textSecondary
                            }
                            Label {
                                text: qsTr("Open an image or project — it will appear here.")
                                font.pixelSize: studio.fontSizeSm
                                color: studio.textMuted
                            }
                            StudioButton {
                                studio: root.studio
                                compact: true
                                primary: true
                                iconName: "upload"
                                text: qsTr("Open image…")
                                onClicked: root.openImageRequested()
                            }
                        }
                    }

                    ListView {
                        anchors.fill: parent
                        visible: converter.recentFiles.length > 0
                        orientation: ListView.Horizontal
                        spacing: studio.spacingMd
                        clip: true
                        model: converter.recentFiles
                        delegate: WelcomeRecentCard {
                            required property var modelData
                            studio: root.studio
                            fileName: modelData.name
                            filePath: modelData.path
                            thumbnailUrl: modelData.thumbnailUrl ?? ""
                            onOpenRequested: root.recentFileRequested(modelData.path)
                        }
                    }
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: studio.spacingMd

                Label {
                    text: qsTr("News & updates").toUpperCase()
                    font.pixelSize: 10
                    font.weight: Font.DemiBold
                    font.letterSpacing: 1.2
                    color: studio.textMuted
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: newsRow.implicitHeight + studio.spacingLg * 2
                    radius: studio.radiusLg
                    color: Qt.rgba(32/255, 32/255, 32/255, 0.65)
                    border.width: 1
                    border.color: Qt.rgba(255, 255, 255, 0.06)

                    RowLayout {
                        id: newsRow
                        anchors.fill: parent
                        anchors.margins: studio.spacingLg
                        spacing: studio.spacingXl

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: studio.spacingSm
                            Label {
                                Layout.fillWidth: true
                                wrapMode: Text.WordWrap
                                text: qsTr("Release notes and tips will land here.")
                                font.pixelSize: studio.fontSizeSm
                                color: studio.textSecondary
                                lineHeight: 1.4
                            }
                            Label {
                                Layout.fillWidth: true
                                wrapMode: Text.WordWrap
                                text: qsTr("Pin tabs to keep projects after restart.")
                                font.pixelSize: studio.fontSizeXs
                                color: studio.textMuted
                            }
                        }

                        Rectangle {
                            implicitWidth: soonLbl.implicitWidth + studio.spacingMd * 2
                            implicitHeight: 26
                            radius: studio.radiusPill
                            color: studio.accentSoft
                            Label {
                                id: soonLbl
                                anchors.centerIn: parent
                                text: qsTr("Soon")
                                font.pixelSize: studio.fontSizeXs
                                font.weight: Font.Medium
                                color: studio.accent
                            }
                        }
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.topMargin: studio.spacingSm
                StudioCheck {
                    studio: root.studio
                    text: qsTr("Show welcome screen on startup")
                    checked: appSettings.showWelcomeOnStartup
                    onToggled: appSettings.setShowWelcomeOnStartup(checked)
                }
                Item { Layout.fillWidth: true }
                Label {
                    text: qsTr("Drop image or .pspx project anywhere on this page")
                    font.pixelSize: studio.fontSizeXs
                    color: studio.textMuted
                }
            }
        }
        }
    }
}
