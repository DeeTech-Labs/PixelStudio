import QtQuick
import QtQuick.Controls
import PixelStudio

pragma Translator: Workspace


PixelPanel {
    id: root
    title: qsTr("Palette")

    readonly property var swatches: converter.previewPalette
    readonly property bool hasSwatches: swatches.length > 0
    readonly property int footerHeight: 18
    readonly property real gridSpacing: 1
    readonly property real minCell: 3
    readonly property real maxCell: 20

    function computeGridLayout(count, availW, availH) {
        if (count <= 0 || availW <= 0 || availH <= 0)
            return { cols: 1, rows: 1, cell: 8 }

        const sp = gridSpacing
        const minC = minCell
        const maxC = maxCell
        let best = { cols: 1, rows: count, cell: minC }

        const maxCols = Math.min(count, 32)
        for (let cols = 1; cols <= maxCols; ++cols) {
            const rows = Math.ceil(count / cols)
            const cellW = (availW - (cols - 1) * sp) / cols
            const cellH = (availH - (rows - 1) * sp) / rows
            const cell = Math.min(maxC, Math.min(cellW, cellH))
            if (cell >= minC && cell >= best.cell)
                best = { cols: cols, rows: rows, cell: cell }
        }
        return best
    }

    Item {
        id: gridHost
        anchors.fill: parent
        anchors.bottomMargin: footerHeight

        readonly property int count: root.swatches.length
        readonly property var layout: root.computeGridLayout(count, width, height)

        Grid {
            id: swatchGrid
            anchors.centerIn: parent
            visible: root.hasSwatches
            columns: gridHost.layout.cols
            rowSpacing: root.gridSpacing
            columnSpacing: root.gridSpacing

            Repeater {
                model: root.swatches
                delegate: Rectangle {
                    required property var modelData
                    required property int index
                    width: gridHost.layout.cell
                    height: gridHost.layout.cell
                    color: modelData
                    border.width: 1
                    border.color: studio.border
                }
            }
        }
    }

    Label {
        anchors.centerIn: parent
        anchors.verticalCenterOffset: -8
        visible: !hasSwatches
        text: qsTr("Colors appear after processing")
        font.pixelSize: studio.fontSizeXs
        color: studio.textMuted
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.WordWrap
        width: parent.width - studio.spacingMd * 2
    }

    Label {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 4
        text: hasSwatches
            ? swatches.length + " " + qsTr("colors")
            : "—"
        font.family: studio.fontFamilyMono
        font.pixelSize: studio.fontSizeXs
        color: studio.textMuted
    }
}
