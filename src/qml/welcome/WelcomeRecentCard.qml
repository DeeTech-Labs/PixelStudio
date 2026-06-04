import QtQuick
import QtQuick.Controls

pragma Translator: PixelStudio

Rectangle {
    id: root
    required property var studio
    required property string fileName
    required property string filePath
    property string thumbnailUrl: ""

    signal openRequested()

    readonly property bool hasThumbnail: thumbnailUrl.length > 0

    readonly property url thumbUrl: hasThumbnail ? thumbnailUrl : ""

    width: 220
    height: 72
    radius: studio.radiusLg
    color: hover.hovered ? studio.surfaceRaised : Qt.rgba(38/255, 38/255, 38/255, 0.9)
    border.width: 1
    border.color: hover.hovered ? Qt.rgba(59/255, 158/255, 1, 0.45) : studio.border

    property real lift: hover.hovered ? -2 : 0

    transform: Translate { y: root.lift }
    Behavior on lift { NumberAnimation { duration: 140; easing.type: Easing.OutCubic } }
    Behavior on color { ColorAnimation { duration: 140 } }
    Behavior on border.color { ColorAnimation { duration: 140 } }

    HoverHandler { id: hover }

    Row {
        anchors.fill: parent
        anchors.margins: studio.spacingMd
        spacing: studio.spacingMd

        Rectangle {
            id: thumbFrame
            width: 40
            height: 40
            radius: studio.radiusMd
            anchors.verticalCenter: parent.verticalCenter
            color: "#141820"
            clip: true
            border.width: 1
            border.color: Qt.rgba(255, 255, 255, 0.08)

            Image {
                id: thumb
                anchors.fill: parent
                visible: root.hasThumbnail && status === Image.Ready
                source: root.thumbUrl
                sourceSize: Qt.size(thumbFrame.width * 2, thumbFrame.height * 2)
                fillMode: Image.PreserveAspectCrop
                asynchronous: true
                cache: true
                smooth: true
                mipmap: true
            }

            Rectangle {
                anchors.fill: parent
                visible: root.hasThumbnail && thumb.status === Image.Loading
                color: studio.surfaceInset
            }

            Item {
                anchors.fill: parent
                visible: !root.hasThumbnail
                         || thumb.status === Image.Error
                         || (root.hasThumbnail && thumb.status !== Image.Ready && thumb.status !== Image.Loading)

                Rectangle {
                    anchors.fill: parent
                    color: studio.accentSoft
                    border.width: 1
                    border.color: Qt.rgba(59/255, 158/255, 1, 0.25)
                }
                Label {
                    anchors.centerIn: parent
                    text: root.hasThumbnail ? "\u29C9" : "\u25A1"
                    font.pixelSize: 18
                    color: studio.accent
                }
            }
        }

        Column {
            width: parent.width - 40 - studio.spacingMd * 3
            anchors.verticalCenter: parent.verticalCenter
            spacing: 2
            Label {
                width: parent.width
                text: root.fileName
                font.pixelSize: studio.fontSizeBase
                font.weight: Font.Medium
                color: studio.text
                elide: Text.ElideRight
            }
            Label {
                width: parent.width
                text: root.filePath
                font.family: studio.fontFamilyMono
                font.pixelSize: studio.fontSizeXs
                color: studio.textMuted
                elide: Text.ElideMiddle
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        onClicked: root.openRequested()
    }
}
