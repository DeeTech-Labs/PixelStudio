import QtQuick
import QtQuick.Controls

pragma Translator: PixelStudio


ComboBox {
    id: root
    required property var studio
    property string toolTipText: ""
    property bool _suppressNextOpen: false

    implicitWidth: 120
    implicitHeight: studio.controlHeight
    font.family: studio.fontFamily
    font.pixelSize: studio.fontSizeSm

    HoverHandler { id: hover }
    ToolTip.visible: hover.hovered && toolTipText.length > 0
    ToolTip.text: toolTipText.length > 0 ? toolTipText : qsTr("Click to choose")

    onPressedChanged: {
        if (pressed && comboPopup.visible) {
            _suppressNextOpen = true
            comboPopup.close()
        }
    }

    delegate: ItemDelegate {
        id: itemDelegate
        required property int index
        width: root.width
        height: studio.controlHeight
        hoverEnabled: true
        background: Rectangle {
            color: itemDelegate.highlighted ? studio.surfaceHover
                : (itemDelegate.hovered ? studio.railHover : "transparent")
        }
        contentItem: Text {
            text: root.textAt(index)
            font: root.font
            color: studio.text
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
            leftPadding: studio.spacingMd
        }
    }

    indicator: Rectangle {
        x: root.width - width - studio.spacingXs
        y: (root.height - height) / 2
        width: 28
        height: root.height - 8
        radius: studio.radiusSm
        color: comboPopup.visible || hover.hovered
            ? studio.surfaceHover
            : studio.surfaceInset
        border.width: 1
        border.color: studio.border
        Text {
            anchors.centerIn: parent
            text: comboPopup.visible ? "▴" : "▾"
            font.pixelSize: studio.fontSizeSm
            font.weight: Font.DemiBold
            color: root.enabled ? studio.text : studio.textMuted
        }
    }

    background: Rectangle {
        implicitWidth: 120
        implicitHeight: studio.controlHeight
        radius: studio.radiusMd
        color: root.enabled ? studio.surfaceInput : studio.surfaceInset
        border.width: 1
        border.color: comboPopup.visible ? studio.borderFocus
            : (root.activeFocus ? studio.borderFocus : studio.border)
    }

    contentItem: Text {
        leftPadding: studio.spacingMd
        rightPadding: studio.spacingMd + 32
        text: root.displayText
        font: root.font
        color: root.enabled ? studio.text : studio.textMuted
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    popup: Popup {
        id: comboPopup
        parent: Overlay.overlay
        padding: studio.spacingXs
        modal: false
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        width: Math.max(root.width, 160)
        implicitHeight: Math.min(list.contentHeight + padding * 2, 280)

        onAboutToShow: {
            if (root._suppressNextOpen) {
                close()
                root._suppressNextOpen = false
                return
            }
            const pos = root.mapToItem(comboPopup.parent, 0, root.height)
            x = pos.x
            y = pos.y + 2
        }

        background: Rectangle {
            radius: studio.radiusMd
            color: studio.surfaceRaised
            border.width: 1
            border.color: studio.border
        }

        contentItem: ListView {
            id: list
            clip: true
            implicitHeight: contentHeight
            width: parent.width
            model: root.delegateModel
            currentIndex: root.highlightedIndex
            boundsBehavior: Flickable.StopAtBounds
            ScrollBar.vertical: ScrollBar {
                implicitWidth: studio.scrollBarWidth
                policy: ScrollBar.AsNeeded
            }
        }
    }
}
