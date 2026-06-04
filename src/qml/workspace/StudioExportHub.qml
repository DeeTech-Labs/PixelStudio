import QtQuick
import QtQuick.Layouts
import PixelStudio

pragma Translator: PixelStudio

Item {
    id: root
    required property var studio
    required property var exportBridge

    anchors.fill: parent

    StudioFrame {
        anchors.fill: parent
        studio: root.studio
        title: qsTr("Export")
        subtitle: qsTr("Save projects and generated files")

        StudioScroll {
            anchors.fill: parent
            studio: root.studio

            ColumnLayout {
                width: parent.width
                spacing: studio.spacingLg

                ExportActionsPanel {
                    Layout.fillWidth: true
                    studio: root.studio
                    onNotify: (m) => root.exportBridge.notify(m)
                    onSaveProjectRequested: root.exportBridge.saveProject()
                    onSaveProjectAsRequested: root.exportBridge.saveProjectAs()
                    onSaveCodeRequested: root.exportBridge.saveCode()
                    onSaveBinRequested: root.exportBridge.saveBin()
                }

                StudioSection {
                    Layout.fillWidth: true
                    studio: root.studio
                    title: qsTr("Import and automation")
                    hint: qsTr("Load an existing C array or watch a folder for automatic export.")

                    StudioButton {
                        Layout.fillWidth: true
                        studio: root.studio
                        iconName: "upload"
                        text: qsTr("Import C header…")
                        onClicked: root.exportBridge.importHeader()
                    }
                    StudioButton {
                        Layout.fillWidth: true
                        studio: root.studio
                        text: converter.watchFolderActive
                               ? qsTr("Watch folder: on")
                               : qsTr("Configure watch folder…")
                        onClicked: root.exportBridge.configureWatchFolder()
                    }
                }

                StudioSection {
                    Layout.fillWidth: true
                    studio: root.studio
                    title: qsTr("Batch output")

                    StudioButton {
                        Layout.fillWidth: true
                        studio: root.studio
                        iconName: "folder-open"
                        text: qsTr("Batch export .h…")
                        onClicked: root.exportBridge.batchExport()
                    }
                    StudioButton {
                        Layout.fillWidth: true
                        studio: root.studio
                        text: qsTr("Build sprite atlas…")
                        onClicked: root.exportBridge.buildAtlas()
                    }
                }
            }
        }
    }
}
