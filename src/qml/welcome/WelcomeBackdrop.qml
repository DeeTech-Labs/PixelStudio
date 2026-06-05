import QtQuick

pragma Translator: PixelStudio

Item {
    id: root

    Rectangle {
        anchors.fill: parent
        color: "#0d1117"
    }

    Canvas {
        id: noise
        anchors.fill: parent
        opacity: 0.028
        renderTarget: Canvas.FramebufferObject

        onPaint: {
            const ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)
            const step = 4
            for (let y = 0; y < height; y += step) {
                for (let x = 0; x < width; x += step) {
                    const v = ((x * 13 + y * 7) % 17) / 17
                    ctx.fillStyle = Qt.rgba(1, 1, 1, v * 0.35)
                    ctx.fillRect(x, y, 1, 1)
                }
            }
        }

        Timer {
            id: repaintDebounce
            interval: 120
            repeat: false
            onTriggered: noise.requestPaint()
        }

        Component.onCompleted: requestPaint()
        onWidthChanged: repaintDebounce.restart()
        onHeightChanged: repaintDebounce.restart()
    }

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            orientation: Gradient.Vertical
            GradientStop { position: 0.0; color: Qt.rgba(1, 1, 1, 0.035) }
            GradientStop { position: 0.45; color: "transparent" }
            GradientStop { position: 1.0; color: Qt.rgba(0, 0, 0, 0.28) }
        }
    }
}
