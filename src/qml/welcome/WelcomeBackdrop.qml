import QtQuick

pragma Translator: PixelStudio

Item {
    id: root
    required property var studio

    Rectangle {
        anchors.fill: parent
        color: "#0e1014"
    }

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            orientation: Gradient.Vertical
            GradientStop { position: 0.0; color: "#162033" }
            GradientStop { position: 0.35; color: "transparent" }
            GradientStop { position: 1.0; color: "#080808" }
        }
        opacity: 0.85
    }

    Rectangle {
        width: parent.width * 0.7
        height: parent.width * 0.7
        x: parent.width * 0.55 - width * 0.5
        y: -height * 0.35
        radius: width / 2
        color: studio.accent
        opacity: 0.07
    }

    Rectangle {
        width: parent.width * 0.5
        height: width
        x: -width * 0.35
        y: parent.height * 0.45
        radius: width / 2
        color: "#22D3EE"
        opacity: 0.04
    }

    Canvas {
        anchors.fill: parent
        opacity: 0.35
        onPaint: {
            const ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)
            const g = ctx.createRadialGradient(width * 0.5, height * 0.35, 0, width * 0.5, height * 0.35, width * 0.55)
            g.addColorStop(0, Qt.rgba(59/255, 158/255, 1, 0.14))
            g.addColorStop(1, "transparent")
            ctx.fillStyle = g
            ctx.fillRect(0, 0, width, height)
        }
        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()
    }

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop { position: 0.0; color: Qt.rgba(0, 0, 0, 0.35) }
            GradientStop { position: 0.12; color: "transparent" }
            GradientStop { position: 0.88; color: "transparent" }
            GradientStop { position: 1.0; color: Qt.rgba(0, 0, 0, 0.35) }
        }
    }
}
