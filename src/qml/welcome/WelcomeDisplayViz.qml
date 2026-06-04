import QtQuick
import QtQuick.Controls

pragma Translator: PixelStudio

Item {
    id: root
    required property var studio

    implicitWidth: 280
    implicitHeight: 132

    property int cols: 56
    property int rows: 28
    property var cells: []

    function reseed() {
        const n = cols * rows
        const arr = new Array(n)
        for (let i = 0; i < n; ++i)
            arr[i] = Math.random() > 0.86 ? 1 : 0
        cells = arr
        canvas.requestPaint()
    }

    function tick() {
        if (cells.length === 0) {
            reseed()
            return
        }
        const arr = cells.slice()
        for (let k = 0; k < 32; ++k) {
            const i = Math.floor(Math.random() * arr.length)
            arr[i] = Math.random() > 0.8 ? 1 : 0
        }
        cells = arr
        canvas.requestPaint()
    }

    opacity: 0
    Component.onCompleted: {
        reseed()
        vizFade.start()
    }
    NumberAnimation {
        id: vizFade
        target: root
        property: "opacity"
        from: 0
        to: 1
        duration: 900
        easing.type: Easing.OutCubic
    }

    Timer {
        interval: 70
        running: true
        repeat: true
        onTriggered: root.tick()
    }

    Rectangle {
        anchors.centerIn: parent
        width: parent.width + 40
        height: parent.height + 40
        radius: studio.radiusXl + 12
        color: studio.accent
        opacity: 0.12
    }

    Rectangle {
        id: bezel
        anchors.fill: parent
        radius: studio.radiusXl
        color: "#050608"
        border.width: 1
        border.color: Qt.rgba(255, 255, 255, 0.08)

        Rectangle {
            anchors.fill: parent
            anchors.margins: 1
            radius: studio.radiusXl - 1
            clip: true
            gradient: Gradient {
                GradientStop { position: 0.0; color: "#141820" }
                GradientStop { position: 1.0; color: "#030304" }
            }

            Canvas {
                id: canvas
                anchors.centerIn: parent
                width: parent.width - 16
                height: parent.height - 16
                opacity: 0.95

                onPaint: {
                    const ctx = getContext("2d")
                    ctx.clearRect(0, 0, width, height)
                    const cw = width / root.cols
                    const ch = height / root.rows
                    for (let r = 0; r < root.rows; ++r) {
                        for (let c = 0; c < root.cols; ++c) {
                            if (!root.cells[r * root.cols + c])
                                continue
                            const flicker = 0.65 + ((r + c) % 5) * 0.06
                            ctx.fillStyle = Qt.rgba(96/255, 180/255, 1, flicker)
                            ctx.fillRect(c * cw + 0.5, r * ch + 0.5, Math.max(1.2, cw - 0.6), Math.max(1.2, ch - 0.6))
                        }
                    }
                }
            }

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                height: parent.height * 0.35
                gradient: Gradient {
                    GradientStop { position: 0.0; color: Qt.rgba(255, 255, 255, 0.06) }
                    GradientStop { position: 1.0; color: "transparent" }
                }
            }
        }

        Rectangle {
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.margins: 10
            implicitWidth: tag.implicitWidth + 12
            implicitHeight: 20
            radius: studio.radiusPill
            color: Qt.rgba(0, 0, 0, 0.55)
            border.width: 1
            border.color: Qt.rgba(59/255, 158/255, 1, 0.35)
            Label {
                id: tag
                anchors.centerIn: parent
                text: "128 × 64"
                font.family: studio.fontFamilyMono
                font.pixelSize: 9
                color: studio.accent
            }
        }
    }

    transform: [
        Rotation { origin.x: bezel.width / 2; origin.y: bezel.height / 2; angle: -2 },
        Scale { origin.x: bezel.width / 2; origin.y: bezel.height / 2; xScale: 1.02; yScale: 1.02 }
    ]
}
