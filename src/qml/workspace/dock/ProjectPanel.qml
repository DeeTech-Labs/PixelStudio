import QtQuick
import QtQuick.Controls
import PixelStudio

pragma Translator: PixelStudio

PixelPanel {
    id: root
    title: qsTr("Project")

    signal assetActivated(string path)

    ListView {
        id: list
        anchors.fill: parent
        anchors.bottomMargin: 22
        clip: true
        spacing: 1
        model: converter.projectAssets

        delegate: ItemDelegate {
            required property var modelData
            required property int index
            width: list.width
            height: 22
            highlighted: modelData.path === converter.sourceFilePath
            background: Rectangle {
                color: parent.highlighted ? studio.accentSoft
                    : (parent.hovered ? studio.railHover : "transparent")
                border.width: parent.highlighted ? 1 : 0
                border.color: studio.accent
            }
            contentItem: Label {
                text: "   " + modelData.name
                font.family: studio.fontFamily
                font.pixelSize: studio.fontSizeXs
                color: parent.highlighted ? studio.accent : studio.textSecondary
                elide: Text.ElideRight
            }
            onClicked: {
                if (modelData.path)
                    root.assetActivated(modelData.path)
            }
        }
    }

    Label {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 20
        padding: studio.spacingXs
        text: converter.projectFile.toString().length > 0
            ? converter.projectName + ".pspx"
            : qsTr("No project")
        font.family: studio.fontFamilyMono
        font.pixelSize: studio.fontSizeXs
        color: studio.textMuted
        elide: Text.ElideMiddle
        verticalAlignment: Text.AlignVCenter
        background: Rectangle {
            color: studio.surface
            border.width: 1
            border.color: studio.border
        }
    }
}
