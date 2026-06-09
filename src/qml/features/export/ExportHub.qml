import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import PixelStudio

pragma Translator: Inspector


Item {
    id: root

    required property var studio
    signal notify(string message)

    implicitHeight: column.implicitHeight
    Layout.fillWidth: true
    width: parent ? parent.width : implicitWidth

    ColumnLayout {
        id: column
        width: root.width
        spacing: studio.spacingLg

        ExportActionsPanel {
            Layout.fillWidth: true
            studio: root.studio
            compact: true
            onNotify: (m) => root.notify(m)
            onSaveProjectRequested: WorkflowRouter.saveProject()
            onSaveProjectAsRequested: WorkflowRouter.saveProjectAs()
            onSaveCodeRequested: WorkflowRouter.saveCode()
            onSaveBinRequested: WorkflowRouter.saveBin()
        }

        InspectorSection {
            studio: root.studio
            title: qsTr("Import and automation")
            hint: qsTr("Load an existing C array or watch a folder for automatic export.")

            StudioButton {
                Layout.fillWidth: true
                studio: root.studio
                text: qsTr("Import C header…")
                onClicked: WorkflowRouter.importHeader()
            }
            StudioButton {
                Layout.fillWidth: true
                studio: root.studio
                text: qsTr("Configure watch folder…")
                onClicked: WorkflowRouter.configureWatchFolder()
            }
        }

        InspectorSection {
            studio: root.studio
            title: qsTr("Batch tools")
            hint: qsTr("Export multiple images or build a sprite atlas.")

            StudioButton {
                Layout.fillWidth: true
                studio: root.studio
                text: qsTr("Batch export .h…")
                onClicked: WorkflowRouter.batchExport()
            }
            StudioButton {
                Layout.fillWidth: true
                studio: root.studio
                text: qsTr("Build sprite atlas…")
                onClicked: WorkflowRouter.buildAtlas()
            }
        }
    }
}
