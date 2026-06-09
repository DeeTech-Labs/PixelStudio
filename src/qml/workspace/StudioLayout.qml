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

    readonly property bool showSource: viewMode === StudioViewMode.Dual || viewMode === StudioViewMode.Source
    readonly property bool showOutput: viewMode === StudioViewMode.Dual || viewMode === StudioViewMode.Output

    readonly property string outputSubtitle: {
        if (!workspace.image.hasPreview)
            return workspace.output.displayWidth + " × " + workspace.output.displayHeight
        let s = workspace.output.displayWidth + " × " + workspace.output.displayHeight
        s += " · " + workspace.output.encodingModeName
        if (workspace.output.dataByteCount > 0)
            s += " · " + workspace.output.dataByteCount + " B"
        return s
    }

    readonly property string sourceSubtitle: {
        if (!workspace.image.hasImage)
            return ""
        return workspace.image.sourceWidth + " × " + workspace.image.sourceHeight
    }

    readonly property bool compactDisplay: workspace.output.displayWidth * workspace.output.displayHeight <= 16384

    Rectangle {
        visible: workspace.exportPanel.batchRunning || workspace.exportPanel.batchProgress > 0
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
                text: workspace.exportPanel.batchRunning ? qsTr("Batch export…") : qsTr("Batch finished")
                font.pixelSize: studio.fontSizeSm
                color: studio.text
            }
            Item { Layout.fillWidth: true }
            ProgressBar { Layout.preferredWidth: 160; from: 0; to: 100; value: workspace.exportPanel.batchProgress }
            Label {
                text: workspace.exportPanel.batchProgress + "%"
                font.family: studio.fontFamilyMono
                font.pixelSize: studio.fontSizeXs
                color: studio.textMuted
            }
            StudioButton {
                visible: workspace.exportPanel.batchRunning
                studio: root.studio
                compact: true
                text: qsTr("Cancel")
                onClicked: workspace.exportPanel.cancelBatchExport()
            }
        }
    }

    SplitView {
        anchors.fill: parent
        anchors.topMargin: (workspace.exportPanel.batchRunning || workspace.exportPanel.batchProgress > 0) ? 36 : 0
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
                    badge: workspace.image.hasImage ? "" : qsTr("empty")

                    StudioViewport {
                        anchors.fill: parent
                        studio: root.studio
                        visible: workspace.image.hasImage && !workspace.image.imageLoading
                        allowUpscale: true
                        imageSource: workspace.image.sourcePath
                        overlayTopLeft: workspace.image.sourceWidth + " × " + workspace.image.sourceHeight
                    }
                    Label {
                        anchors.centerIn: parent
                        visible: workspace.image.imageLoading
                        text: qsTr("Loading image…")
                        color: studio.textMuted
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap
                        width: Math.max(80, parent.width - studio.spacing2xl * 2)
                    }
                    StudioDropCanvas {
                        anchors.fill: parent
                        studio: root.studio
                        visible: !workspace.image.hasImage && !workspace.image.imageLoading
                        onFileDropped: (url) => workspace.image.loadImage(url)
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
                        visible: workspace.image.hasPreview
                        allowUpscale: true
                        showGrid: workspace.viewport.showGrid
                        autoPixelGrid: false
                        imageSource: workspace.image.previewPath
                        overlayTopLeft: workspace.output.displayWidth + " × " + workspace.output.displayHeight
                        overlayTopRight: workspace.output.encodingModeName
                    }
                    Label {
                        anchors.centerIn: parent
                        visible: workspace.image.imageLoading
                        text: qsTr("Loading image…")
                        color: studio.textMuted
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap
                        width: Math.max(80, parent.width - studio.spacing2xl * 2)
                    }
                    Label {
                        anchors.centerIn: parent
                        visible: !workspace.image.imageLoading && !workspace.image.hasPreview
                        text: workspace.image.hasImage ? qsTr("Rasterizing…") : qsTr("Process an image to see the result here.")
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
            visible: workspace.settings.showSidebar
            SplitView.preferredWidth: workspace.settings.showSidebar
                ? Math.min(studio.inspectorWidth, root.width * 0.34)
                : 0
            SplitView.minimumWidth: workspace.settings.showSidebar ? 220 : 0
            SplitView.maximumWidth: workspace.settings.showSidebar ? 400 : 0
            studio: root.studio
            onNotify: (m) => root.notify(m)
        }
    }
}
