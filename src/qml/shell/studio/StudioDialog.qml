import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

pragma Translator: Shell


Dialog {
    id: root

    required property var studio

    property bool showCornerBrackets: true
    property bool frameDoubleBorder: false
    property int frameBorderWidth: studio.pixelBorderWidth
    property int frameRadius: studio.radiusSm
    property color headerTitleColor: studio.text
    property int headerTitleElide: Text.ElideRight

    padding: studio.spacingXl
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    function centerInOverlay() {
        if (!parent || width <= 0 || height <= 0)
            return
        x = Math.max(0, Math.round((parent.width - width) / 2))
        y = Math.max(0, Math.round((parent.height - height) / 2))
    }

    onOpened: centerInOverlay()
    onWidthChanged: if (visible) Qt.callLater(centerInOverlay)
    onHeightChanged: if (visible) Qt.callLater(centerInOverlay)

    Connections {
        target: root.parent
        function onWidthChanged() {
            if (root.visible)
                root.centerInOverlay()
        }
        function onHeightChanged() {
            if (root.visible)
                root.centerInOverlay()
        }
    }

    background: Item {
        clip: true

        Rectangle {
            anchors.fill: parent
            radius: root.frameRadius
            color: studio.canvas
            border.width: root.frameBorderWidth
            border.color: studio.accent
        }

        Rectangle {
            visible: root.frameDoubleBorder
            anchors.fill: parent
            anchors.margins: 4
            radius: root.frameRadius
            color: "transparent"
            border.width: 1
            border.color: studio.accent
            opacity: 0.85
        }

        component CornerBracket: Item {
            id: bracket
            width: 12
            height: 12
            property color tint: studio.accent
            property bool flipH: false
            property bool flipV: false

            transform: Scale {
                xScale: bracket.flipH ? -1 : 1
                yScale: bracket.flipV ? -1 : 1
                origin.x: bracket.flipH ? bracket.width : 0
                origin.y: bracket.flipV ? bracket.height : 0
            }

            Rectangle { width: 10; height: 1; color: bracket.tint }
            Rectangle { width: 1; height: 8; color: bracket.tint }
            Rectangle { x: 8; width: 2; height: 1; color: bracket.tint }
            Rectangle { y: 6; width: 1; height: 2; color: bracket.tint }
        }

        CornerBracket {
            visible: root.showCornerBrackets
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.margins: -1
        }
        CornerBracket {
            visible: root.showCornerBrackets
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.margins: -1
            flipH: true
        }
        CornerBracket {
            visible: root.showCornerBrackets
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.margins: -1
            flipV: true
        }
        CornerBracket {
            visible: root.showCornerBrackets
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            anchors.margins: -1
            flipH: true
            flipV: true
        }
    }

    header: Item {
        implicitHeight: studio.frameHeaderHeight + 1

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: studio.spacingMd
            anchors.rightMargin: studio.spacingSm
            spacing: studio.spacingSm

            Label {
                text: root.title
                font.family: studio.fontFamilyPixel
                font.pixelSize: studio.fontSizePixel
                color: root.headerTitleColor
                elide: root.headerTitleElide
                Layout.fillWidth: true
            }

            Button {
                flat: true
                text: "\u00d7"
                font.pixelSize: studio.fontSizePixel
                font.family: studio.fontFamilyPixel
                implicitWidth: 28
                implicitHeight: 28
                Layout.alignment: Qt.AlignVCenter
                onClicked: root.close()
                palette.buttonText: studio.accent
                HoverHandler { id: closeHover }
                background: Rectangle {
                    radius: studio.radiusSm
                    color: closeHover.hovered ? studio.accentSoft : "transparent"
                }
            }
        }

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: 1
            color: studio.accent
            opacity: 0.55
        }
    }
}
