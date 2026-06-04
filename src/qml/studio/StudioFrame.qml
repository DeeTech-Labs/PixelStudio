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

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: studio.frameHeaderHeight
            color: studio.surface
            topLeftRadius: studio.radiusMd
            topRightRadius: studio.radiusMd
            Rectangle {
                anchors.bottom: parent.bottom
                width: parent.width
                height: 1
                color: studio.divider
            }
            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: studio.spacingMd
                anchors.rightMargin: studio.spacingMd
                Label {
                    text: root.title
                    font.family: studio.fontFamily
                    font.pixelSize: studio.fontSizeSm
                    font.weight: Font.DemiBold
                    color: studio.text
                }
                Rectangle {
                    visible: badge.length > 0
                    implicitWidth: badgeLbl.implicitWidth + studio.spacingSm * 2
                    implicitHeight: 18
                    radius: studio.radiusPill
                    color: studio.accentSoft
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
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: studio.previewBackdrop
            bottomLeftRadius: studio.radiusMd
            bottomRightRadius: studio.radiusMd
            clip: true

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
