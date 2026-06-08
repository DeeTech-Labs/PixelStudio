import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import PixelStudio

pragma Translator: Welcome


DropArea {
    id: root
    required property var studio

    signal openImageRequested()
    signal pasteRequested()
    signal openProjectRequested()
    signal newProjectRequested()
    signal continueLastProjectRequested()
    signal recentItemRequested(string path, string tabId)
    signal fileDropped(var urls)
    signal settingsRequested()

    onDropped: (drop) => {
        if (drop.hasUrls && drop.urls.length > 0)
            root.fileDropped(drop.urls)
    }

    readonly property string fontPixel: studio.fontFamilyPixel
    readonly property string fontUi: studio.fontFamily

    readonly property color wBg: studio.background
    readonly property color wCard: studio.surface
    readonly property color wBorder: studio.border
    readonly property color wText: studio.text
    readonly property color wText2: studio.textSecondary
    readonly property color wAccent: studio.accent
    readonly property color wAccentDark: studio.accentDark
    readonly property color wAccentTop: studio.accentTop
    readonly property color wAccentBottom: studio.accentBottom

    readonly property int designWidth: 1100
    readonly property int designHeight: 960
    readonly property real titleStretchY: 1.28
    readonly property int recentMax: 4
    readonly property int recentColumns: 2
    readonly property int recentCardThumb: 80
    readonly property int recentCardHeight: recentCardThumb + 24
    readonly property int recentGridGap: 16
    readonly property var recentWelcomeItems: tabController.welcomeRecentItems
    readonly property int recentCount: Math.min(recentMax, recentWelcomeItems.length)
    readonly property int recentRowCount: recentCount > 0 ? Math.ceil(recentCount / recentColumns) : 0
    readonly property int recentGridHeight: recentCount > 0
        ? recentRowCount * recentCardHeight + Math.max(0, recentRowCount - 1) * recentGridGap
        : 120

    readonly property real pageScale: Math.min(
        1,
        width / designWidth,
        height / designHeight
    )

    WelcomeBackdrop {
        anchors.fill: parent
    }

    Rectangle {
        anchors.fill: parent
        visible: root.containsDrag
        color: Qt.rgba(45/255, 212/255, 191/255, 0.08)
        border.width: 2
        border.color: root.wAccent
        opacity: 0.55
    }

    Item {
        id: viewport
        anchors.fill: parent

        Item {
            id: pageHost
            width: designWidth * pageScale
            height: pageColumn.height * pageScale
            x: (viewport.width - width) / 2
            y: Math.max(12, (viewport.height - height) / 2)

            Column {
                id: pageColumn
                width: designWidth
                spacing: 0
                transformOrigin: Item.TopLeft
                scale: pageScale

                Column {
                    width: parent.width
                    spacing: 20

                    WelcomeBrandMark {
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    Item {
                        width: titleRow.implicitWidth + 80
                        height: titleRow.implicitHeight * root.titleStretchY + 24
                        anchors.horizontalCenter: parent.horizontalCenter

                        Row {
                            id: titleRow
                            spacing: 0
                            anchors.horizontalCenter: parent.horizontalCenter
                            anchors.verticalCenter: parent.verticalCenter
                            transformOrigin: Item.Center

                            transform: Scale {
                                xScale: 1
                                yScale: root.titleStretchY
                                origin.x: titleRow.width / 2
                                origin.y: titleRow.height / 2
                            }

                            Text {
                                text: qsTr("Pixel")
                                font.family: root.fontPixel
                                font.pixelSize: 36
                                color: root.wText
                            }
                            Text {
                                text: qsTr("Studio")
                                font.family: root.fontPixel
                                font.pixelSize: 36
                                color: root.wAccent
                            }
                        }

                        WelcomeSparkle {
                            x: titleRow.x - 40
                            y: titleRow.y + titleRow.height * root.titleStretchY * 0.55
                            tint: root.wAccent
                            armLength: 5
                        }
                        WelcomeSparkle {
                            x: titleRow.x + titleRow.width + 12
                            y: titleRow.y + 4
                            tint: root.wAccent
                            armLength: 6
                            sparkleOpacity: 0.8
                        }
                        WelcomeSparkle {
                            x: titleRow.x + titleRow.width + 28
                            y: titleRow.y + titleRow.height * root.titleStretchY * 0.75
                            tint: Qt.rgba(1, 1, 1, 0.4)
                            armLength: 4
                            sparkleOpacity: 0.45
                        }
                        WelcomeSparkle {
                            x: titleRow.x + 28
                            y: titleRow.y - 16
                            tint: Qt.rgba(1, 1, 1, 0.35)
                            armLength: 4
                            sparkleOpacity: 0.35
                        }
                    }

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: Math.min(parent.width - 48, 720)
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap
                        text: qsTr("Convert pixel art to OLED, TFT and retro displays with ease.")
                        font.family: root.fontUi
                        font.pixelSize: 18
                        lineHeight: 1.5
                        color: root.wText2
                    }

                    Item {
                        id: newProjectWrap
                        width: parent.width
                        height: 66
                        anchors.horizontalCenter: parent.horizontalCenter

                        readonly property int btnWidth: Math.min(480, parent.width - 48)

                        Rectangle {
                            width: newProjectWrap.btnWidth
                            height: 60
                            anchors.horizontalCenter: parent.horizontalCenter
                            anchors.top: parent.top
                            anchors.topMargin: 6
                            radius: 12
                            color: root.wAccentDark
                        }

                        Rectangle {
                            id: newProjectBtn
                            width: newProjectWrap.btnWidth
                            height: 60
                            anchors.horizontalCenter: parent.horizontalCenter
                            anchors.top: parent.top
                            radius: 12
                            color: "transparent"

                            gradient: Gradient {
                                orientation: Gradient.Vertical
                                GradientStop { position: 0.0; color: root.wAccentTop }
                                GradientStop { position: 1.0; color: root.wAccentBottom }
                            }

                            Row {
                                id: newProjectRow
                                anchors.horizontalCenter: parent.horizontalCenter
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.verticalCenterOffset: 5
                                spacing: 12

                                Item {
                                    width: plusText.width
                                    height: projectLabel.height

                                    Text {
                                        id: plusText
                                        anchors.horizontalCenter: parent.horizontalCenter
                                        anchors.verticalCenter: parent.verticalCenter
                                        anchors.verticalCenterOffset: 2
                                        text: "+"
                                        font.family: root.fontPixel
                                        font.pixelSize: 20
                                        color: root.wText
                                    }
                                }

                                Text {
                                    id: projectLabel
                                    text: qsTr("New project")
                                    font.family: root.fontPixel
                                    font.pixelSize: 20
                                    color: root.wText
                                }
                            }

                            MouseArea {
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: root.newProjectRequested()
                            }
                        }
                    }
                }

                Item {
                    width: parent.width
                    height: recentBlock.height + 56

                    Column {
                        id: recentBlock
                        width: parent.width
                        anchors.top: parent.top
                        anchors.topMargin: 56
                        spacing: 24

                        RowLayout {
                            width: parent.width
                            spacing: 8

                            Text {
                                text: qsTr("Recent projects")
                                font.family: root.fontPixel
                                font.pixelSize: 10
                                color: root.wAccent
                            }

                            Item { Layout.fillWidth: true }

                            Item {
                                Layout.preferredWidth: viewAllRow.implicitWidth
                                Layout.preferredHeight: viewAllRow.implicitHeight

                                Row {
                                    id: viewAllRow
                                    spacing: 8

                                    Text {
                                        text: qsTr("View all projects")
                                        font.family: root.fontUi
                                        font.pixelSize: 14
                                        color: viewAllHit.containsMouse ? root.wAccent : root.wText2
                                    }

                                    StudioIcon {
                                        name: "chevron-right"
                                        iconSize: 16
                                        tint: viewAllHit.containsMouse ? root.wAccent : root.wText2
                                    }
                                }

                                MouseArea {
                                    id: viewAllHit
                                    anchors.fill: parent
                                    anchors.margins: -6
                                    hoverEnabled: true
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: root.openProjectRequested()
                                }
                            }
                        }

                        Item {
                            width: parent.width
                            height: root.recentGridHeight

                            GridLayout {
                                id: recentGrid
                                anchors.fill: parent
                                columns: root.recentColumns
                                columnSpacing: root.recentGridGap
                                rowSpacing: root.recentGridGap
                                visible: root.recentCount > 0

                                Repeater {
                                    model: root.recentCount
                                    delegate: WelcomeRecentCard {
                                        required property int index
                                        Layout.fillWidth: true
                                        Layout.preferredWidth: 0
                                        Layout.preferredHeight: root.recentCardHeight
                                        thumbSize: root.recentCardThumb
                                        studio: root.studio
                                        fontPixel: root.fontPixel
                                        fontUi: root.fontUi
                                    fileName: root.recentWelcomeItems[index].name ?? ""
                                    filePath: root.recentWelcomeItems[index].path ?? ""
                                    thumbnailUrl: root.recentWelcomeItems[index].thumbnailUrl ?? ""
                                    projectMeta: root.recentWelcomeItems[index].projectMeta ?? ""
                                    dateTimeText: root.recentWelcomeItems[index].modifiedText ?? ""
                                    onOpenRequested: {
                                        const item = root.recentWelcomeItems[index]
                                        root.recentItemRequested(item.path ?? "", item.tabId ?? "")
                                    }
                                    }
                                }
                            }

                            Rectangle {
                                anchors.fill: parent
                                visible: root.recentCount === 0
                                radius: 12
                                color: root.wCard
                                border.width: 1
                                border.color: root.wBorder
                                Text {
                                    anchors.centerIn: parent
                                    text: qsTr("No recent projects yet")
                                    font.family: root.fontUi
                                    font.pixelSize: 14
                                    color: root.wText2
                                }
                            }
                        }
                    }
                }

                Item {
                    width: parent.width
                    height: 56
                }

                Rectangle {
                    width: parent.width
                    height: 1
                    color: root.wBorder
                }

                Item {
                    width: parent.width
                    height: 40
                }

                WelcomeFooterNav {
                    width: parent.width
                    height: 52
                    studio: root.studio
                    fontUi: root.fontUi
                    iconSize: 40
                    accentColor: root.wAccent
                    titleColor: root.wText
                    subtitleColor: root.wText2
                    onItemActivated: (key) => {
                        if (key === "settings")
                            root.settingsRequested()
                    }
                }

                Item {
                    width: parent.width
                    height: 12
                }
            }
        }
    }
}
