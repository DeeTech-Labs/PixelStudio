import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import PixelStudio

pragma Translator: Inspector


Item {
    id: root
    required property var studio
    required property var win
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
            onSaveProjectRequested: root.win.saveProject()
            onSaveProjectAsRequested: root.win.saveProjectAs()
            onSaveCodeRequested: root.win.saveCode()
            onSaveBinRequested: root.win.saveBin()
        }

        StudioSection {
            studio: root.studio
            title: qsTr("Import and automation")
            hint: qsTr("Load an existing C array or watch a folder for automatic export.")

            StudioButton {
                Layout.fillWidth: true
                studio: root.studio
                compact: true
                text: qsTr("Import C header…")
                onClicked: root.win.importHeader()
            }
            StudioButton {
                Layout.fillWidth: true
                studio: root.studio
                compact: true
                text: exporter.watchFolderActive
                       ? qsTr("Watch folder: on")
                       : qsTr("Configure watch folder…")
                onClicked: root.win.configureWatchFolder()
            }
        }

        StudioSection {
            studio: root.studio
            title: qsTr("Batch output")

            StudioButton {
                Layout.fillWidth: true
                studio: root.studio
                compact: true
                text: qsTr("Batch export .h…")
                onClicked: root.win.batchExport()
            }
            StudioButton {
                Layout.fillWidth: true
                studio: root.studio
                compact: true
                text: qsTr("Build sprite atlas…")
                onClicked: root.win.buildAtlas()
            }
        }
    }
}
