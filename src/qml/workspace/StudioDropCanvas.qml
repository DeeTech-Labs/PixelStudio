import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import PixelStudio

pragma Translator: PixelStudio

Item {
    id: root
    required property var studio
    signal fileDropped(url fileUrl)
    signal openRequested()
    signal pasteRequested()

    property bool containsDrag: dropArea.containsDrag

    DropArea {
        id: dropArea
        anchors.fill: parent
        onDropped: (drop) => {
            if (drop.hasUrls && drop.urls.length > 0)
                root.fileDropped(drop.urls[0])
        }
    }

    Rectangle {
        anchors.fill: parent
        anchors.margins: studio.spacing2xl
        radius: studio.radiusLg
        color: containsDrag ? studio.accentSoft : "transparent"
        border.width: 2
        border.color: containsDrag ? studio.accent : studio.border
        opacity: containsDrag ? 1 : 0.85

        ColumnLayout {
            anchors.centerIn: parent
            width: Math.min(parent.width - studio.spacing2xl * 2, 360)
            spacing: studio.spacingLg

            Item {
                Layout.alignment: Qt.AlignHCenter
                width: 64
                height: 64
                StudioIcon {
                    anchors.centerIn: parent
                    name: "upload"
                    iconSize: 40
                    tint: containsDrag ? studio.accent : studio.textSecondary
                }
            }

            Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
                text: containsDrag
                    ? qsTr("Release to import")
                    : qsTr("Drop an image here, or open a file")
                font.family: studio.fontFamily
                font.pixelSize: studio.fontSizeLg
                font.weight: Font.Medium
                color: studio.text
            }

            Label {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                text: qsTr("PNG · JPG · BMP · GIF · WebP · clipboard (Ctrl+V)")
                font.pixelSize: studio.fontSizeXs
                color: studio.textMuted
            }

            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                spacing: studio.spacingSm
                StudioButton {
                    studio: root.studio
                    primary: true
                    iconName: "upload"
                    text: qsTr("Open image…")
                    onClicked: root.openRequested()
                }
                StudioButton {
                    studio: root.studio
                    iconName: "clipboard"
                    text: qsTr("Paste")
                    onClicked: root.pasteRequested()
                }
            }
        }
    }
}
