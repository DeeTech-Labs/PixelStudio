import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import PixelStudio

pragma Translator: Workspace


Item {
    id: root
    required property var studio
    signal fileDropped(url fileUrl)
    signal openRequested()
    signal pasteRequested()

    property bool containsDrag: dropArea.containsDrag
    readonly property bool compactLayout: width < 340

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
        color: containsDrag ? studio.accentSoft : "transparent"
        opacity: containsDrag ? 1 : 0
        Behavior on opacity { NumberAnimation { duration: studio.durationFast } }
    }

    ColumnLayout {
        anchors.centerIn: parent
        width: Math.max(120, Math.min(parent.width - studio.spacingXl * 2, 400))
        spacing: compactLayout ? studio.spacingMd : studio.spacingLg

        Item {
            Layout.alignment: Qt.AlignHCenter
            width: compactLayout ? 48 : 64
            height: width
            StudioIcon {
                anchors.centerIn: parent
                name: "upload"
                iconSize: compactLayout ? 32 : 40
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
            font.pixelSize: compactLayout ? studio.fontSizeBase : studio.fontSizeLg
            font.weight: Font.Medium
            color: studio.text
        }

        Label {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
            text: qsTr("PNG · JPG · BMP · GIF · WebP · clipboard (Ctrl+V)")
            font.pixelSize: studio.fontSizeXs
            color: studio.textMuted
        }

        RowLayout {
            visible: !compactLayout
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter
            spacing: studio.spacingSm
            StudioButton {
                Layout.fillWidth: true
                studio: root.studio
                primary: true
                compact: true
                iconName: "upload"
                text: qsTr("Open image…")
                onClicked: root.openRequested()
            }
            StudioButton {
                Layout.fillWidth: true
                studio: root.studio
                compact: true
                iconName: "clipboard"
                text: qsTr("Paste")
                onClicked: root.pasteRequested()
            }
        }

        ColumnLayout {
            visible: compactLayout
            Layout.fillWidth: true
            spacing: studio.spacingSm
            StudioButton {
                Layout.fillWidth: true
                studio: root.studio
                primary: true
                compact: true
                iconName: "upload"
                text: qsTr("Open image…")
                onClicked: root.openRequested()
            }
            StudioButton {
                Layout.fillWidth: true
                studio: root.studio
                compact: true
                iconName: "clipboard"
                text: qsTr("Paste")
                onClicked: root.pasteRequested()
            }
        }
    }
}
