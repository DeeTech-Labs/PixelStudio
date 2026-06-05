import QtQuick
import QtQuick.Controls

pragma Translator: PixelStudio

Dialog {
    id: root

    required property var studio

    padding: studio.spacingLg

    background: Rectangle {
        radius: studio.radiusLg
        color: studio.surface
        border.width: studio.pixelBorderWidth
        border.color: studio.accent
    }

    header: Label {
        text: root.title
        font.family: studio.fontFamilyPixel
        font.pixelSize: studio.fontSizePixel
        color: studio.accent
        padding: studio.spacingSm
    }
}
