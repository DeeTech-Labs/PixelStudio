import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

pragma Translator: PixelStudio

Item {
    id: root
    default property alias body: bodySlot.data
    property alias footer: footerSlot.data

    required property var studio
    property string title: ""
    property string subtitle: ""
    property string badge: ""

    Rectangle {
        id: chrome
        anchors.fill: parent
        radius: studio.radiusMd
        color: studio.previewBackdrop
        border.width: studio.pixelBorderWidth
        border.color: studio.border
        clip: true

        ColumnLayout {
            anchors.fill: parent
            spacing: 0

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: studio.frameHeaderHeight
                color: studio.surface

                Rectangle {
                    anchors.top: parent.top
                    width: parent.width
                    height: 2
                    color: studio.accent
                    opacity: 0.85
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: studio.spacingSm
                    anchors.rightMargin: studio.spacingSm

                    Label {
                        text: root.title.toUpperCase()
                        font.family: studio.fontFamilyPixel
                        font.pixelSize: studio.fontSizePixel
                        color: studio.accent
                    }

                    Rectangle {
                        visible: badge.length > 0
                        radius: studio.radiusSm
                        implicitWidth: badgeLbl.implicitWidth + studio.spacingSm * 2
                        implicitHeight: 16
                        color: studio.accentSoft
                        border.width: 1
                        border.color: studio.accent
                        Label {
                            id: badgeLbl
                            anchors.centerIn: parent
                            text: root.badge
                            font.pixelSize: studio.fontSizeXs
                            color: studio.accent
                        }
                    }

                    Item { Layout.fillWidth: true }

                    Label {
                        visible: subtitle.length > 0
                        text: root.subtitle
                        font.family: studio.fontFamilyMono
                        font.pixelSize: studio.fontSizeXs
                        color: studio.textMuted
                    }
                }

                Rectangle {
                    anchors.bottom: parent.bottom
                    width: parent.width
                    height: 1
                    color: studio.border
                }
            }

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true

                Item {
                    id: bodySlot
                    anchors.fill: parent
                    anchors.bottomMargin: footerSlot.height > 0 ? footerSlot.height : 0
                }

                Item {
                    id: footerSlot
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    implicitHeight: childrenRect.height
                    height: implicitHeight
                }
            }
        }
    }
}
