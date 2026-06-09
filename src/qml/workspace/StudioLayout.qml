import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import PixelStudio

pragma Translator: Workspace


Item {
    id: root

    required property var studio
    required property var win
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
            return displayOutput.displayWidth + " × " + displayOutput.displayHeight
        let s = displayOutput.displayWidth + " × " + displayOutput.displayHeight
        s += " · " + displayOutput.encodingModeName
        if (displayOutput.dataByteCount > 0)
            s += " · " + displayOutput.dataByteCount + " B"
        return s
    }

    readonly property string sourceSubtitle: {
        if (!converter.hasImage)
            return ""
        return converter.sourceWidth + " × " + converter.sourceHeight
    }

    readonly property bool compactDisplay: displayOutput.displayWidth * displayOutput.displayHeight <= 16384

    Rectangle {
        visible: exporter.batchRunning || exporter.batchProgress > 0
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
                text: exporter.batchRunning ? qsTr("Batch export…") : qsTr("Batch finished")
                font.pixelSize: studio.fontSizeSm
                color: studio.text
            }
            Item { Layout.fillWidth: true }
            ProgressBar { Layout.preferredWidth: 160; from: 0; to: 100; value: exporter.batchProgress }
            Label {
                text: exporter.batchProgress + "%"
                font.family: studio.fontFamilyMono
                font.pixelSize: studio.fontSizeXs
                color: studio.textMuted
            }
            StudioButton {
                visible: exporter.batchRunning
                studio: root.studio
                compact: true
                text: qsTr("Cancel")
                onClicked: exporter.cancelBatchExport()
            }
        }
    }

    SplitView {
        anchors.fill: parent
        anchors.topMargin: (exporter.batchRunning || exporter.batchProgress > 0) ? 36 : 0
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
                        visible: converter.hasImage && !converter.imageLoading
                        allowUpscale: true
                        imageSource: converter.sourcePath
                        overlayTopLeft: converter.sourceWidth + " × " + converter.sourceHeight
                    }
                    Label {
                        anchors.centerIn: parent
                        visible: converter.imageLoading
                        text: qsTr("Loading image…")
                        color: studio.textMuted
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap
                        width: Math.max(80, parent.width - studio.spacing2xl * 2)
                    }
                    StudioDropCanvas {
                        anchors.fill: parent
                        studio: root.studio
                        visible: !converter.hasImage && !converter.imageLoading
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
                        showGrid: viewport.showGrid
                        autoPixelGrid: false
                        imageSource: converter.previewPath
                        overlayTopLeft: displayOutput.displayWidth + " × " + displayOutput.displayHeight
                        overlayTopRight: displayOutput.encodingModeName
                    }
                    Label {
                        anchors.centerIn: parent
                        visible: converter.imageLoading
                        text: qsTr("Loading image…")
                        color: studio.textMuted
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap
                        width: Math.max(80, parent.width - studio.spacing2xl * 2)
                    }
                    Label {
                        anchors.centerIn: parent
                        visible: !converter.imageLoading && !converter.hasPreview
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
                win: root.win
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
            win: root.win
            onNotify: (m) => root.notify(m)
        }
    }
}
