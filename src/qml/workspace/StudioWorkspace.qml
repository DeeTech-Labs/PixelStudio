import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import PixelStudio

pragma Translator: PixelStudio

Item {
    id: root
    required property var studio
    property int viewMode: 0
    property var onOpenRequested: null
    property var exportBridge: null
    signal viewModeRequested(int mode)
    signal pasteRequested()

    readonly property string outputSubtitle: {
        if (!converter.hasPreview)
            return converter.displayWidth + " × " + converter.displayHeight
        let s = converter.displayWidth + " × " + converter.displayHeight
        s += " · " + converter.encodingModeName
        if (converter.dataByteCount > 0)
            s += " · " + converter.dataByteCount + " B"
        return s
    }

    readonly property string sourceSubtitle: {
        if (!converter.hasImage)
            return ""
        return converter.sourceWidth + " × " + converter.sourceHeight
    }

    readonly property bool compactDisplay: converter.displayWidth * converter.displayHeight <= 16384

    Rectangle {
        visible: converter.batchRunning || converter.batchProgress > 0
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: studio.spacingSm
        height: 28
        radius: studio.radiusMd
        color: studio.accentSoft
        border.width: 1
        border.color: studio.accentMuted
        z: 10

        RowLayout {
            anchors.fill: parent
            anchors.margins: studio.spacingSm
            Label {
                text: converter.batchRunning ? qsTr("Batch export…") : qsTr("Batch finished")
                font.pixelSize: studio.fontSizeSm
                color: studio.text
            }
            Item { Layout.fillWidth: true }
            ProgressBar { Layout.preferredWidth: 160; from: 0; to: 100; value: converter.batchProgress }
            Label {
                text: converter.batchProgress + "%"
                font.family: studio.fontFamilyMono
                font.pixelSize: studio.fontSizeXs
                color: studio.textMuted
            }
            StudioButton {
                visible: converter.batchRunning
                studio: root.studio
                compact: true
                text: qsTr("Cancel")
                onClicked: converter.cancelBatchExport()
            }
        }
    }

    Item {
        id: stage
        anchors.fill: parent
        anchors.topMargin: (converter.batchRunning || converter.batchProgress > 0) ? 36 : 0

        RowLayout {
            id: viewBar
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.margins: studio.spacingSm
            spacing: 2
            z: 5
            visible: converter.hasImage

            Repeater {
                model: [
                    { mode: 0, icon: "columns-2", tip: qsTr("Dual view") },
                    { mode: 1, icon: "image", tip: qsTr("Source") },
                    { mode: 3, icon: "file-image", tip: qsTr("Output") }
                ]
                delegate: StudioIconButton {
                    required property var modelData
                    studio: root.studio
                    small: true
                    iconName: modelData.icon
                    toggled: root.viewMode === modelData.mode
                    toolTipText: modelData.tip
                    onClicked: root.viewModeRequested(modelData.mode)
                }
            }
        }

        RowLayout {
            anchors.fill: parent
            anchors.topMargin: viewBar.visible ? viewBar.implicitHeight + studio.spacingSm : 0
            spacing: studio.spacingSm
            visible: viewMode === 0

            Loader { Layout.fillWidth: true; Layout.fillHeight: true; sourceComponent: sourcePanel }
            Loader { Layout.fillWidth: true; Layout.fillHeight: true; sourceComponent: outputPanel }
        }

        Loader {
            anchors.fill: parent
            anchors.topMargin: viewBar.visible ? viewBar.implicitHeight + studio.spacingSm : 0
            visible: viewMode === 1
            sourceComponent: sourcePanel
        }
        Loader {
            anchors.fill: parent
            anchors.topMargin: viewBar.visible ? viewBar.implicitHeight + studio.spacingSm : 0
            visible: viewMode === 3
            sourceComponent: outputPanel
        }
    }

    Component {
        id: sourcePanel
        StudioFrame {
            studio: root.studio
            anchors.fill: parent
            title: qsTr("Source")
            subtitle: root.sourceSubtitle
            badge: converter.hasImage ? "" : qsTr("empty")

            StudioViewport {
                anchors.fill: parent
                studio: root.studio
                visible: converter.hasImage
                allowUpscale: true
                imageSource: converter.sourcePath
                overlayTopLeft: converter.sourceWidth + " × " + converter.sourceHeight
            }
            StudioDropCanvas {
                anchors.fill: parent
                studio: root.studio
                visible: !converter.hasImage
                onFileDropped: (url) => converter.loadImage(url)
                onOpenRequested: if (root.onOpenRequested) root.onOpenRequested()
                onPasteRequested: root.pasteRequested()
            }
        }
    }

    Component {
        id: outputPanel
        StudioFrame {
            studio: root.studio
            anchors.fill: parent
            title: qsTr("Display output")
            subtitle: root.outputSubtitle

            StudioViewport {
                anchors.fill: parent
                studio: root.studio
                visible: converter.hasPreview
                allowUpscale: true
                showGrid: converter.showGrid
                autoPixelGrid: root.compactDisplay
                gridThresholdZoom: converter.gridThresholdZoom
                imageSource: converter.previewPath
                overlayTopLeft: converter.displayWidth + " × " + converter.displayHeight
                overlayTopRight: converter.encodingModeName
            }
            Label {
                anchors.centerIn: parent
                visible: !converter.hasPreview
                text: converter.hasImage ? qsTr("Rasterizing…") : qsTr("Import an image to start")
                color: studio.textMuted
            }
        }
    }
}
