import QtQuick
import QtQuick.Layouts
import PixelStudio

pragma Translator: Welcome


RowLayout {
    id: root
    required property var studio
    required property string fontUi
    required property color accentColor
    required property color titleColor
    required property color subtitleColor
    property int iconSize: 40

    signal itemActivated(string key)

    readonly property var items: [
        { key: "docs", icon: "book" },
        { key: "examples", icon: "palette" },
        { key: "profiles", icon: "chip" },
        { key: "settings", icon: "settings" }
    ]

    spacing: 0

    Repeater {
        model: root.items
        delegate: Item {
            required property var modelData
            required property int index
            Layout.fillWidth: true
            Layout.preferredHeight: Math.max(root.iconSize + 8, 52)

            readonly property string languageTag: appSettings.languageCode

            Rectangle {
                visible: index > 0
                width: 1
                height: Math.min(36, parent.height * 0.7)
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                color: Qt.rgba(1, 1, 1, 0.07)
            }

            MouseArea {
                id: hit
                anchors.fill: parent
                anchors.leftMargin: index > 0 ? 1 : 0
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: root.itemActivated(modelData.key)

                Row {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.leftMargin: index > 0 ? 16 : 8
                    anchors.rightMargin: 8
                    spacing: 15

                    Item {
                        width: root.iconSize
                        height: root.iconSize
                        anchors.verticalCenter: parent.verticalCenter

                        StudioIcon {
                            anchors.centerIn: parent
                            name: modelData.icon
                            iconSize: root.iconSize
                            tint: root.accentColor
                        }
                    }

                    Column {
                        width: parent.width - root.iconSize - 15
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 4

                        Text {
                            width: parent.width
                            text: {
                                const _lang = languageTag
                                switch (modelData.key) {
                                case "docs":
                                    return qsTranslate("Welcome", "Documentation")
                                case "examples":
                                    return qsTranslate("Welcome", "Examples")
                                case "profiles":
                                    return qsTranslate("Welcome", "Display Profiles")
                                case "settings":
                                    return qsTranslate("Welcome", "Settings")
                                }
                                return ""
                            }
                            font.family: root.fontUi
                            font.pixelSize: 16
                            font.weight: Font.DemiBold
                            color: root.titleColor
                            elide: Text.ElideRight
                            maximumLineCount: 1
                        }

                        Text {
                            width: parent.width
                            text: {
                                const _lang = languageTag
                                switch (modelData.key) {
                                case "docs":
                                    return qsTranslate("Welcome", "Learn the basics")
                                case "examples":
                                    return qsTranslate("Welcome", "Explore sample projects")
                                case "profiles":
                                    return qsTranslate("Welcome", "Manage your displays")
                                case "settings":
                                    return qsTranslate("Welcome", "Customize PixelStudio")
                                }
                                return ""
                            }
                            font.family: root.fontUi
                            font.pixelSize: 14
                            color: root.subtitleColor
                            elide: Text.ElideRight
                            maximumLineCount: 1
                        }
                    }
                }

                Rectangle {
                    anchors.fill: parent
                    color: hit.containsMouse ? Qt.rgba(1, 1, 1, 0.03) : "transparent"
                    z: -1
                }
            }
        }
    }
}
