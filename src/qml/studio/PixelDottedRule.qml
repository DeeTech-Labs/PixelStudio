import QtQuick

pragma Translator: PixelStudio

Canvas {
    id: root

    required property color lineColor
    property bool vertical: false

    onWidthChanged: requestPaint()
    onHeightChanged: requestPaint()
    onLineColorChanged: requestPaint()
    onVerticalChanged: requestPaint()
    Component.onCompleted: requestPaint()

    onPaint: {
        const ctx = getContext("2d")
        ctx.clearRect(0, 0, width, height)
        ctx.strokeStyle = root.lineColor
        ctx.globalAlpha = 0.55
        ctx.lineWidth = 1
        ctx.setLineDash([2, 5])
        ctx.beginPath()
        if (vertical) {
            ctx.moveTo(0.5, 0)
            ctx.lineTo(0.5, height)
        } else {
            ctx.moveTo(0, 0.5)
            ctx.lineTo(width, 0.5)
        }
        ctx.stroke()
    }
}
