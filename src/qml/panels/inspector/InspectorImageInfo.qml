import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import PixelStudio

pragma Translator: Inspector


InspectorSection {
    id: section
    title: qsTr("Image info")
    hint: qsTr("Metadata for the active source image.")

    GridLayout {
        Layout.fillWidth: true
        columns: 2
        columnSpacing: section.studio.spacingMd
        rowSpacing: section.studio.spacingXs

        Label { text: qsTr("Size"); font.pixelSize: section.studio.fontSizeXs; color: section.studio.textMuted }
        Label {
            text: inspectorImage.hasImage
                ? inspectorImage.sourceWidth + " × " + inspectorImage.sourceHeight
                : "—"
            font.family: section.studio.fontFamilyMono
            font.pixelSize: section.studio.fontSizeXs
            color: section.studio.text
        }
        Label { text: qsTr("Colors"); font.pixelSize: section.studio.fontSizeXs; color: section.studio.textMuted }
        Label {
            text: inspectorImage.previewColorCount > 0 ? inspectorImage.previewColorCount : "—"
            font.family: section.studio.fontFamilyMono
            font.pixelSize: section.studio.fontSizeXs
            color: section.studio.text
        }
        Label { text: qsTr("BPP"); font.pixelSize: section.studio.fontSizeXs; color: section.studio.textMuted }
        Label {
            text: inspectorImage.hasPreview ? inspectorImage.output.encodingModeName : "—"
            font.family: section.studio.fontFamilyMono
            font.pixelSize: section.studio.fontSizeXs
            color: section.studio.text
        }
        Label { text: qsTr("Format"); font.pixelSize: section.studio.fontSizeXs; color: section.studio.textMuted }
        Label {
            text: inspectorImage.imageFormatName.length > 0 ? inspectorImage.imageFormatName : "—"
            font.family: section.studio.fontFamilyMono
            font.pixelSize: section.studio.fontSizeXs
            color: section.studio.text
        }
    }
}
