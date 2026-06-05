import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import PixelStudio

pragma Translator: PixelStudio

PixelFrame {
    id: root

    required property var exportBridge
    signal saveCode()
    signal saveBin()

    title: qsTr("Code")

    CodeSyntaxColorsDialog {
        id: syntaxColorsDialog
        studio: root.studio
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: studio.spacingSm
        spacing: studio.spacingSm

        RowLayout {
            Layout.fillWidth: true
            spacing: studio.spacingXs

            Item { Layout.fillWidth: true }

            StudioIconButton {
                studio: root.studio
                small: true
                iconName: "copy"
                toolTipText: qsTr("Copy")
                enabled: converter.generatedCode.length > 0
                onClicked: converter.copyToClipboard(converter.generatedCode)
            }
            StudioIconButton {
                studio: root.studio
                small: true
                iconName: "download"
                toolTipText: qsTr("Save .h")
                enabled: converter.generatedCode.length > 0
                onClicked: root.saveCode()
            }
            StudioIconButton {
                studio: root.studio
                small: true
                iconName: "file-image"
                toolTipText: qsTr("Save .bin")
                enabled: converter.hasPreview
                onClicked: root.saveBin()
            }
            StudioIconButton {
                studio: root.studio
                small: true
                iconName: "palette"
                toolTipText: qsTr("Syntax colors")
                onClicked: syntaxColorsDialog.open()
            }
            StudioIconButton {
                studio: root.studio
                small: true
                iconName: "settings"
                toolTipText: qsTr("Code options")
                onClicked: exportBridge.openExportHub()
            }
            StudioIconButton {
                studio: root.studio
                small: true
                iconName: "columns-2"
                toolTipText: converter.showFullGeneratedCode ? qsTr("Preview") : qsTr("Show all")
                visible: converter.generatedCodeTruncated
                onClicked: converter.setShowFullGeneratedCode(!converter.showFullGeneratedCode)
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: studio.radiusMd
            color: studio.surfaceInset
            border.width: studio.pixelBorderWidth
            border.color: studio.border
            clip: true

            ScrollView {
                anchors.fill: parent
                anchors.margins: studio.spacingXs
                clip: true
                CodeSyntaxView {
                    anchors.fill: parent
                    studio: root.studio
                    sourceText: converter.generatedCodePreview
                    wrap: appSettings.codeWrap
                    placeholderText: qsTr("// Code appears here after you import an image.")
                }
            }
        }
    }
}
