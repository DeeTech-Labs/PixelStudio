import QtQuick
import PixelStudio

pragma Translator: PixelStudio

Item {
    id: root

    required property var studio
    signal assetActivated(string path)

    implicitWidth: studio.leftDockWidth

    Column {
        anchors.fill: parent
        spacing: studio.spacingSm

        PalettePanel {
            width: parent.width
            height: 160
            studio: root.studio
        }

        ProjectPanel {
            width: parent.width
            height: Math.max(100, parent.height - 160 - studio.spacingSm)
            studio: root.studio
            onAssetActivated: (path) => root.assetActivated(path)
        }
    }
}
