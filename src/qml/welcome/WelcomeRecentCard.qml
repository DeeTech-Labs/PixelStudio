import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import PixelStudio

pragma Translator: PixelStudio

Rectangle {
    id: root
    required property var studio
    required property string fontPixel
    required property string fontUi
    required property string fileName
    required property string filePath
    property string thumbnailUrl: ""
    property string projectMeta: ""
    property string dateTimeText: ""

    property int thumbSize: 80
    property int outerPadding: 12
    property int innerSpacing: 12

    signal openRequested()

    readonly property bool hasThumbnail: thumbnailUrl.length > 0
    readonly property url thumbUrl: hasThumbnail ? thumbnailUrl : ""
    readonly property bool hasProjectMeta: projectMeta.length > 0
    readonly property bool hasDateTime: dateTimeText.length > 0

    readonly property color cardBg: studio.surface
    readonly property color cardBorder: studio.border
    readonly property color accent: studio.accent
    readonly property color metaColor: studio.textSecondary

    implicitHeight: thumbSize + outerPadding * 2
    radius: studio.radiusLg
    color: cardBg
    border.width: 1
    border.color: hover.hovered ? accent : studio.accent
    clip: false

    HoverHandler { id: hover }

    RowLayout {
        anchors.fill: parent
        anchors.margins: outerPadding
        spacing: innerSpacing

        Rectangle {
            id: preview
            Layout.preferredWidth: thumbSize
            Layout.preferredHeight: thumbSize
            Layout.alignment: Qt.AlignVCenter
            radius: studio.radiusMd
            color: "#0d1117"
            clip: true
            border.width: 1
            border.color: cardBorder

            Image {
                id: thumb
                anchors.fill: parent
                visible: root.hasThumbnail && status === Image.Ready
                source: root.thumbUrl
                sourceSize: Qt.size(preview.width * 2, preview.height * 2)
                fillMode: Image.PreserveAspectCrop
                asynchronous: true
                cache: true
                smooth: false
                mipmap: true
            }

            Rectangle {
                anchors.fill: parent
                visible: root.hasThumbnail && thumb.status === Image.Loading
                color: Qt.rgba(1, 1, 1, 0.03)
            }

            Rectangle {
                anchors.fill: parent
                visible: !root.hasThumbnail
                         || thumb.status === Image.Error
                         || (root.hasThumbnail && thumb.status !== Image.Ready && thumb.status !== Image.Loading)
                color: Qt.rgba(45/255, 212/255, 191/255, 0.06)
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            RowLayout {
                Layout.fillWidth: true
                spacing: 6

                Text {
                    Layout.fillWidth: true
                    text: root.fileName
                    font.family: root.fontUi
                    font.pixelSize: 14
                    font.weight: Font.DemiBold
                    color: "#ffffff"
                    elide: Text.ElideRight
                    maximumLineCount: 1
                }

                Item {
                    Layout.preferredWidth: 22
                    Layout.preferredHeight: 22
                    Layout.alignment: Qt.AlignTop

                    StudioIcon {
                        anchors.centerIn: parent
                        name: "dots-vertical"
                        iconSize: 18
                        tint: hover.hovered ? root.accent : root.metaColor
                    }

                    MouseArea {
                        anchors.fill: parent
                        anchors.margins: -4
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: root.openRequested()
                    }
                }
            }

            Text {
                Layout.fillWidth: true
                Layout.topMargin: 4
                text: root.projectMeta
                visible: root.hasProjectMeta
                font.family: root.fontUi
                font.pixelSize: 12
                color: metaColor
                elide: Text.ElideRight
                maximumLineCount: 1
            }

            Item { Layout.fillHeight: true }

            RowLayout {
                Layout.fillWidth: true
                visible: root.hasDateTime
                spacing: 8

                StudioIcon {
                    name: "calendar"
                    iconSize: 14
                    tint: metaColor
                }

                Text {
                    Layout.fillWidth: true
                    text: root.dateTimeText
                    font.family: root.fontUi
                    font.pixelSize: 12
                    color: metaColor
                    elide: Text.ElideRight
                    maximumLineCount: 1
                }
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        onClicked: root.openRequested()
        z: -1
    }
}
