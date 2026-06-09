import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import PixelStudio

pragma Translator: Inspector


Item {
    id: root
    required property var studio
    property bool compact: false
    signal notify(string message)

    signal saveProjectRequested()
    signal saveProjectAsRequested()
    signal saveCodeRequested()
    signal saveBinRequested()
    signal batchExportRequested()
    signal buildAtlasRequested()
    signal importHeaderRequested()
    signal configureWatchFolderRequested()

    implicitHeight: column.implicitHeight
    Layout.fillWidth: true
    width: parent ? parent.width : implicitWidth

    ColumnLayout {
        id: column
        width: root.width
        spacing: studio.spacingLg

        Label {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            font.pixelSize: studio.fontSizeSm
            color: studio.textMuted
            text: qsTr("Save the generated C array or project. Full export tools are in the Export view (Ctrl+Shift+E).")
        }

        StudioSection {
            studio: root.studio
            title: qsTr("Generated output")
            hint: qsTr("Copy or save the firmware array.")

            StudioButton {
                Layout.fillWidth: true
                studio: root.studio
                compact: root.compact
                primary: true
                iconName: "download"
                text: qsTr("Save code to file…")
                enabled: workspace.image.generatedCode.length > 0
                onClicked: root.saveCodeRequested()
            }
            StudioButton {
                Layout.fillWidth: true
                studio: root.studio
                compact: root.compact
                iconName: "copy"
                text: qsTr("Copy generated code")
                enabled: workspace.image.generatedCode.length > 0
                onClicked: {
                    workspace.exportPanel.copyToClipboard(workspace.image.generatedCode)
                    root.notify(qsTr("Copied to clipboard"))
                }
            }
            StudioButton {
                Layout.fillWidth: true
                studio: root.studio
                compact: root.compact
                iconName: "clipboard"
                text: qsTr("Copy array only")
                enabled: workspace.image.generatedCode.length > 0
                onClicked: {
                    workspace.code.copyGeneratedArray()
                    root.notify(qsTr("Array copied to clipboard"))
                }
            }
            StudioButton {
                Layout.fillWidth: true
                studio: root.studio
                compact: root.compact
                iconName: "file-image"
                text: qsTr("Save binary…")
                enabled: workspace.image.hasPreview
                onClicked: root.saveBinRequested()
            }
        }

        StudioSection {
            studio: root.studio
            title: qsTr("Project")
            hint: qsTr("Save workflow as .pspx")

            StudioButton {
                Layout.fillWidth: true
                studio: root.studio
                compact: root.compact
                iconName: "save"
                text: qsTr("Save project")
                enabled: workspace.project.projectFile.toString().length > 0
                onClicked: root.saveProjectRequested()
            }
            StudioButton {
                Layout.fillWidth: true
                studio: root.studio
                compact: root.compact
                iconName: "save"
                text: qsTr("Save project as…")
                onClicked: root.saveProjectAsRequested()
            }
        }
    }
}
