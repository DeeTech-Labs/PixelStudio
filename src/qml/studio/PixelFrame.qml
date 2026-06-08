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
                    spacing: studio.spacingXs

                    Label {
                        Layout.fillWidth: root.subtitle.length === 0
                        Layout.maximumWidth: root.subtitle.length > 0
                            ? Math.min(implicitWidth, parent.width * 0.42)
                            : implicitWidth
                        text: root.title.toUpperCase()
                        font.family: studio.fontFamilyPixel
                        font.pixelSize: studio.fontSizePixel
                        color: studio.accent
                        elide: Text.ElideRight
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
                            elide: Text.ElideRight
                        }
                    }

                    Item { Layout.fillWidth: true; Layout.minimumWidth: studio.spacingXs }

                    Label {
                        visible: subtitle.length > 0
                        Layout.fillWidth: true
                        Layout.minimumWidth: 0
                        horizontalAlignment: Text.AlignRight
                        text: root.subtitle
                        font.family: studio.fontFamilyMono
                        font.pixelSize: studio.fontSizeXs
                        color: studio.textMuted
                        elide: Text.ElideLeft
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
