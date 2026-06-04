import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import PixelStudio

pragma Translator: PixelStudio

Item {
    id: root
    required property var studio
    property url imageSource
    property real zoomLevel: 1.0
    property bool allowUpscale: true
    property bool showGrid: false
    property int gridThresholdZoom: 8
    property bool panEnabled: true
    property string overlayTopLeft: ""
    property string overlayTopRight: ""
    property bool autoPixelGrid: false
    property int autoGridMaxPixels: 16384
    property int autoGridThresholdZoom: 4

    property real panX: 0
    property real panY: 0
    property real panOverscroll: 0.4

    readonly property bool effectiveShowGrid: showGrid
        || (autoPixelGrid && imgW > 0 && imgH > 0 && imgW * imgH <= autoGridMaxPixels)
    readonly property int effectiveGridThreshold: effectiveShowGrid && autoPixelGrid && !showGrid
        ? autoGridThresholdZoom
        : gridThresholdZoom

    readonly property var _zoomSteps: [0.125, 0.25, 0.5, 0.75, 1, 1.5, 2, 3, 4, 8, 16]

    property real imgW: img.status === Image.Ready ? img.sourceSize.width : 1
    property real imgH: img.status === Image.Ready ? img.sourceSize.height : 1
    property real displayW: imgW * zoomLevel
    property real displayH: imgH * zoomLevel

    function maxZoom() { return allowUpscale ? 16 : Math.max(1, fitZoom()) }

    function fitZoom() {
        if (imgW < 1 || imgH < 1 || viewport.width < 1 || viewport.height < 1)
            return 1
        const sx = viewport.width / imgW
        const sy = viewport.height / imgH
        return Math.min(sx, sy, allowUpscale ? 16 : 1)
    }

    function panMarginX() {
        return Math.max(64, viewport.width * panOverscroll)
    }

    function panMarginY() {
        return Math.max(64, viewport.height * panOverscroll)
    }

    function clampPan() {
        if (viewport.width < 1 || viewport.height < 1)
            return
        const vw = viewport.width
        const vh = viewport.height
        const dw = displayW
        const dh = displayH
        const mx = panMarginX()
        const my = panMarginY()

        if (dw <= vw)
            panX = Math.max(-mx, Math.min(vw - dw + mx, panX))
        else
            panX = Math.max(vw - dw - mx, Math.min(mx, panX))

        if (dh <= vh)
            panY = Math.max(-my, Math.min(vh - dh + my, panY))
        else
            panY = Math.max(vh - dh - my, Math.min(my, panY))
    }

    function centerContent() {
        panX = (viewport.width - displayW) / 2
        panY = (viewport.height - displayH) / 2
        clampPan()
    }

    function zoomByFactor(factor, anchorX, anchorY) {
        const oldZoom = zoomLevel
        const cap = maxZoom()
        const next = Math.max(0.125, Math.min(cap, oldZoom * factor))
        if (Math.abs(next - oldZoom) < 0.00001)
            return

        const ax = anchorX !== undefined ? anchorX : viewport.width / 2
        const ay = anchorY !== undefined ? anchorY : viewport.height / 2
        const ratio = next / oldZoom

        panX = ax - (ax - panX) * ratio
        panY = ay - (ay - panY) * ratio
        zoomLevel = next
        clampPan()
    }

    function zoomTo(level, anchorX, anchorY) {
        const oldZoom = zoomLevel
        const cap = maxZoom()
        const next = Math.max(0.125, Math.min(cap, level))
        if (Math.abs(next - oldZoom) < 0.0001)
            return

        const ax = anchorX !== undefined ? anchorX : viewport.width / 2
        const ay = anchorY !== undefined ? anchorY : viewport.height / 2
        const ratio = next / oldZoom

        panX = ax - (ax - panX) * ratio
        panY = ay - (ay - panY) * ratio
        zoomLevel = next
        clampPan()
    }

    function zoomInAt(ax, ay) {
        for (let i = 0; i < _zoomSteps.length; ++i) {
            if (zoomLevel < _zoomSteps[i] - 0.001) {
                zoomTo(_zoomSteps[i], ax, ay)
                return
            }
        }
    }

    function zoomOutAt(ax, ay) {
        for (let i = _zoomSteps.length - 1; i >= 0; --i) {
            if (zoomLevel > _zoomSteps[i] + 0.001) {
                zoomTo(_zoomSteps[i], ax, ay)
                return
            }
        }
    }

    function zoomIn() { zoomInAt(viewport.width / 2, viewport.height / 2) }
    function zoomOut() { zoomOutAt(viewport.width / 2, viewport.height / 2) }
    function fitToView() { zoomTo(fitZoom()) }

    function scrollByWheel(deltaY, deltaX) {
        panY += deltaY
        panX += deltaX
        clampPan()
    }

    readonly property string zoomLabel: {
        const pct = Math.round(zoomLevel * 100)
        if (!allowUpscale && Math.abs(zoomLevel - fitZoom()) < 0.02)
            return qsTr("Fit")
        return pct + "%"
    }

    FocusScope {
        id: viewport
        anchors.fill: parent
        focus: true

        property bool shiftKeyDown: false
        property bool ctrlKeyDown: false

        Keys.onPressed: (event) => {
            if (event.key === Qt.Key_Shift)
                shiftKeyDown = true
            if (event.key === Qt.Key_Control)
                ctrlKeyDown = true
        }
        Keys.onReleased: (event) => {
            if (event.key === Qt.Key_Shift)
                shiftKeyDown = false
            if (event.key === Qt.Key_Control)
                ctrlKeyDown = false
        }

        HoverHandler {
            acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
            onActiveChanged: {
                if (active)
                    viewport.forceActiveFocus()
            }
        }

        Canvas {
            anchors.fill: parent
            z: -1
            onPaint: {
                const ctx = getContext("2d")
                ctx.reset()
                const sz = 12
                for (let y = 0; y < height; y += sz) {
                    for (let x = 0; x < width; x += sz) {
                        const odd = ((x / sz) + (y / sz)) % 2
                        ctx.fillStyle = odd ? studio.checkerA : studio.checkerB
                        ctx.fillRect(x, y, sz, sz)
                    }
                }
            }
        }

        Item {
            id: contentLayer
            z: 0
            x: root.panX
            y: root.panY
            width: root.displayW
            height: root.displayH

            Image {
                id: img
                anchors.fill: parent
                source: root.imageSource
                fillMode: Image.Stretch
                smooth: false
                cache: false
            }

            Canvas {
                anchors.fill: parent
                visible: root.effectiveShowGrid && root.zoomLevel >= root.effectiveGridThreshold
                onPaint: {
                    const ctx = getContext("2d")
                    ctx.reset()
                    ctx.strokeStyle = studio.border
                    ctx.lineWidth = 1
                    const pw = Math.max(1, width / Math.max(1, root.imgW))
                    const ph = Math.max(1, height / Math.max(1, root.imgH))
                    for (let x = 0; x <= width; x += pw) {
                        ctx.beginPath()
                        ctx.moveTo(x + 0.5, 0)
                        ctx.lineTo(x + 0.5, height)
                        ctx.stroke()
                    }
                    for (let y = 0; y <= height; y += ph) {
                        ctx.beginPath()
                        ctx.moveTo(0, y + 0.5)
                        ctx.lineTo(width, y + 0.5)
                        ctx.stroke()
                    }
                }
            }
        }

        MouseArea {
            id: panArea
            z: 1
            anchors.fill: parent
            enabled: root.panEnabled
            acceptedButtons: Qt.LeftButton
            hoverEnabled: true
            propagateComposedEvents: false
            preventStealing: true
            cursorShape: pressed ? Qt.ClosedHandCursor : Qt.OpenHandCursor

            property real dragPanX: 0
            property real dragPanY: 0
            property real dragMouseX: 0
            property real dragMouseY: 0

            onPressed: {
                dragPanX = root.panX
                dragPanY = root.panY
                dragMouseX = mouseX
                dragMouseY = mouseY
            }
            onPositionChanged: {
                if (!pressed)
                    return
                root.panX = dragPanX + (mouseX - dragMouseX)
                root.panY = dragPanY + (mouseY - dragMouseY)
                root.clampPan()
            }
            onContainsMouseChanged: {
                if (containsMouse)
                    viewport.forceActiveFocus()
            }
        }

        WheelHandler {
            id: wheelHandler
            onWheel: (event) => {
                event.accepted = true
                const shift = viewport.shiftKeyDown || (event.modifiers & Qt.ShiftModifier)
                const ctrl = viewport.ctrlKeyDown || (event.modifiers & Qt.ControlModifier)

                if (shift && !ctrl) {
                    root.scrollByWheel(event.angleDelta.y * 0.75, 0)
                    return
                }
                if (ctrl) {
                    const hDelta = event.angleDelta.x !== 0 ? event.angleDelta.x : event.angleDelta.y
                    root.scrollByWheel(0, hDelta * 0.75)
                    return
                }

                const pt = wheelHandler.point.position
                const ax = pt ? pt.x : event.x
                const ay = pt ? pt.y : event.y
                const factor = Math.pow(1.12, event.angleDelta.y / 120.0)
                root.zoomByFactor(factor, ax, ay)
            }
        }
    }

    RowLayout {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: studio.spacingSm
        spacing: studio.spacingSm
        z: 2
        visible: root.overlayTopLeft.length > 0 || root.overlayTopRight.length > 0

        Rectangle {
            visible: root.overlayTopLeft.length > 0
            implicitWidth: overlayLeftLabel.implicitWidth + studio.spacingMd
            height: studio.frameFooterHeight - 4
            radius: studio.radiusMd
            color: studio.floatingPanel
            border.width: 1
            border.color: studio.border
            Label {
                id: overlayLeftLabel
                anchors.centerIn: parent
                text: root.overlayTopLeft
                font.family: studio.fontFamilyMono
                font.pixelSize: studio.fontSizeXs
                color: studio.textSecondary
            }
        }
        Rectangle {
            visible: root.overlayTopRight.length > 0
            implicitWidth: overlayRightLabel.implicitWidth + studio.spacingMd
            height: studio.frameFooterHeight - 4
            radius: studio.radiusMd
            color: studio.floatingPanel
            border.width: 1
            border.color: studio.border
            Label {
                id: overlayRightLabel
                anchors.centerIn: parent
                text: root.overlayTopRight
                font.family: studio.fontFamilyMono
                font.pixelSize: studio.fontSizeXs
                color: studio.textMuted
            }
        }
    }

    RowLayout {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: studio.spacingSm
        spacing: 2
        z: 2

        Rectangle {
            Layout.preferredHeight: studio.frameFooterHeight - 4
            implicitWidth: zoomRow.implicitWidth + studio.spacingMd
            radius: studio.radiusMd
            color: studio.floatingPanel
            border.width: 1
            border.color: studio.border

            RowLayout {
                id: zoomRow
                anchors.centerIn: parent
                spacing: 0

                StudioIconButton {
                    studio: root.studio
                    small: true
                    iconText: "−"
                    onClicked: root.zoomOut()
                }
                Label {
                    text: root.zoomLabel
                    font.family: studio.fontFamilyMono
                    font.pixelSize: studio.fontSizeXs
                    color: studio.textSecondary
                    Layout.minimumWidth: 48
                    horizontalAlignment: Text.AlignHCenter
                }
                StudioIconButton {
                    studio: root.studio
                    small: true
                    iconText: "+"
                    onClicked: root.zoomIn()
                }
                Rectangle { width: 1; height: 16; color: studio.divider }
                StudioIconButton {
                    studio: root.studio
                    small: true
                    text: "1:1"
                    onClicked: root.zoomTo(1)
                }
                StudioIconButton {
                    studio: root.studio
                    small: true
                    text: qsTr("Fit")
                    onClicked: root.fitToView()
                }
            }
        }
    }

    onImageSourceChanged: Qt.callLater(fitToView)
    onDisplayWChanged: clampPan()
    onDisplayHChanged: clampPan()
    Component.onCompleted: Qt.callLater(fitToView)
    onWidthChanged: Qt.callLater(centerContent)
    onHeightChanged: Qt.callLater(centerContent)
}
