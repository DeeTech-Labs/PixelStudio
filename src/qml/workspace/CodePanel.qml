import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import PixelStudio

pragma Translator: Workspace


PixelFrame {
    id: root

    required property var win
    signal saveCode()
    signal saveBin()

    title: qsTr("Code")

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
                enabled: workspace.image.generatedCode.length > 0
                onClicked: workspace.exportPanel.copyToClipboard(workspace.image.generatedCode)
            }
            StudioIconButton {
                studio: root.studio
                small: true
                iconName: "download"
                toolTipText: qsTr("Save .h")
                enabled: workspace.image.generatedCode.length > 0
                onClicked: root.saveCode()
            }
            StudioIconButton {
                studio: root.studio
                small: true
                iconName: "file-image"
                toolTipText: qsTr("Save .bin")
                enabled: workspace.image.hasPreview
                onClicked: root.saveBin()
            }
            StudioIconButton {
                studio: root.studio
                small: true
                iconName: "palette"
                toolTipText: qsTr("Syntax colors")
                onClicked: WorkflowRouter.openSettingsTab(1)
            }
            StudioIconButton {
                studio: root.studio
                small: true
                iconName: "settings"
                toolTipText: qsTr("Code options")
                onClicked: win.openExportHub()
            }
            StudioIconButton {
                studio: root.studio
                small: true
                iconName: "columns-2"
                toolTipText: workspace.code.showFullGeneratedCode ? qsTr("Preview") : qsTr("Show all")
                visible: workspace.image.generatedCodeTruncated
                onClicked: workspace.code.setShowFullGeneratedCode(!workspace.code.showFullGeneratedCode)
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

            CodeSyntaxView {
                anchors.fill: parent
                anchors.margins: studio.spacingXs
                studio: root.studio
                sourceText: workspace.image.generatedCodePreview
                wrap: workspace.settings.codeWrap
                placeholderText: qsTr("// Code appears here after you import an image.")
            }
        }
    }
}
