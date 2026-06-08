import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import PixelStudio

pragma Translator: PixelStudio

Item {
    id: root

    required property var studio
    required property var exportBridge
    property int viewMode: 0
    property var onOpenRequested: null
    signal pasteRequested()
    signal viewModeRequested(int mode)
    signal assetOpenRequested(string path)
    signal saveCodeRequested()
    signal saveBinRequested()
    signal notify(string message)

    property alias inspectorPageIndex: inspectorDock.pageIndex

    readonly property bool showSource: viewMode === 0 || viewMode === 1
    readonly property bool showOutput: viewMode === 0 || viewMode === 3

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
        color: studio.accentSoft
        border.width: 1
        border.color: studio.accent
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

    SplitView {
        anchors.fill: parent
        anchors.topMargin: (converter.batchRunning || converter.batchProgress > 0) ? 36 : 0
        orientation: Qt.Horizontal

        handle: Rectangle {
            implicitWidth: studio.splitHandleSize
            implicitHeight: studio.splitHandleSize
            color: SplitHandle.pressed ? studio.accent
                : (SplitHandle.hovered ? studio.border : "transparent")
        }

        LeftAssetDock {
            SplitView.preferredWidth: Math.min(studio.leftDockWidth, root.width * 0.22)
            SplitView.minimumWidth: 140
            SplitView.maximumWidth: 280
            studio: root.studio
            onAssetActivated: (path) => root.assetOpenRequested(path)
        }

        SplitView {
            SplitView.fillWidth: true
            orientation: Qt.Vertical

            handle: Rectangle {
                implicitWidth: studio.splitHandleSize
                implicitHeight: studio.splitHandleSize
                color: SplitHandle.pressed ? studio.accent
                    : (SplitHandle.hovered ? studio.border : "transparent")
            }

            RowLayout {
                SplitView.fillHeight: true
                SplitView.minimumHeight: 160
                spacing: studio.spacingSm

                PixelFrame {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.preferredWidth: showSource ? 1 : 0
                    visible: showSource
                    studio: root.studio
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

                PixelFrame {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.preferredWidth: showOutput ? 1 : 0
                    visible: showOutput
                    studio: root.studio
                    title: qsTr("Output")
                    subtitle: root.outputSubtitle

                    StudioViewport {
                        anchors.fill: parent
                        studio: root.studio
                        visible: converter.hasPreview
                        allowUpscale: true
                        showGrid: appSettings.showPixelGrid
                        autoPixelGrid: false
                        imageSource: converter.previewPath
                        overlayTopLeft: converter.displayWidth + " × " + converter.displayHeight
                        overlayTopRight: converter.encodingModeName
                    }
                    Label {
                        anchors.centerIn: parent
                        visible: !converter.hasPreview
                        text: converter.hasImage ? qsTr("Rasterizing…") : qsTr("Process an image to see the result here.")
                        color: studio.textMuted
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap
                        width: Math.max(80, parent.width - studio.spacing2xl * 2)
                    }
                }
            }

            CodePanel {
                id: codePanel
                SplitView.fillWidth: true
                SplitView.preferredHeight: studio.codeDockHeight
                SplitView.minimumHeight: 100
                studio: root.studio
                exportBridge: root.exportBridge
                onSaveCode: root.saveCodeRequested()
                onSaveBin: root.saveBinRequested()
            }
        }

        InspectorDock {
            id: inspectorDock
            visible: appSettings.showSidebar
            SplitView.preferredWidth: appSettings.showSidebar
                ? Math.min(studio.inspectorWidth, root.width * 0.34)
                : 0
            SplitView.minimumWidth: appSettings.showSidebar ? 220 : 0
            SplitView.maximumWidth: appSettings.showSidebar ? 400 : 0
            studio: root.studio
            exportBridge: root.exportBridge
            onNotify: (m) => root.notify(m)
        }
    }
}
