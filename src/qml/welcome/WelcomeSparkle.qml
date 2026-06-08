import QtQuick

pragma Translator: Welcome


Item {
    id: root
    property color tint: "#26A69A"
    property real sparkleOpacity: 0.75
    property int armLength: 4

    width: armLength * 2 + 3
    height: width
    opacity: sparkleOpacity

    Canvas {
        anchors.fill: parent
        onPaint: {
            const ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)
            ctx.strokeStyle = root.tint
            ctx.lineWidth = 2
            ctx.lineCap = "square"
            const mid = width / 2
            ctx.beginPath()
            ctx.moveTo(mid, 1)
            ctx.lineTo(mid, height - 1)
            ctx.moveTo(1, mid)
            ctx.lineTo(width - 1, mid)
            ctx.stroke()
        }
    }
}
