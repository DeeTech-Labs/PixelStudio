import QtQuick
import QtQuick.Controls

pragma Translator: PixelStudio

Item {
    id: root

    required property var studio
    property string label: ""
    property bool selected: false
    property bool closable: false
    property bool compact: false

    signal clicked()
    signal closeClicked()

    implicitWidth: compact
        ? Math.max(40, tabLabel.implicitWidth + studio.spacingSm * 2)
        : Math.max(72, labelRow.implicitWidth + studio.spacingMd * 2)
    implicitHeight: compact ? 24 : 28

    Rectangle {
        anchors.fill: parent
        radius: studio.radiusSm
        color: selected ? studio.surfaceRaised : (mouse.containsMouse ? studio.railHover : studio.surfaceInset)
        border.width: studio.pixelBorderWidth
        border.color: selected ? studio.accent : studio.border
    }

    Rectangle {
        visible: selected
        anchors.top: parent.top
        width: parent.width
        height: 2
        color: studio.accent
    }

    Row {
        id: labelRow
        anchors.centerIn: parent
        spacing: studio.spacingXs

        Label {
            id: tabLabel
            width: compact && root.width > 0
                ? Math.max(0, root.width - studio.spacingSm * 2 - (closable ? 20 : 0))
                : implicitWidth
            text: label
            font.family: selected ? studio.fontFamilyPixel : studio.fontFamily
            font.pixelSize: compact
                ? studio.fontSizeXs
                : (selected ? studio.fontSizePixel : studio.fontSizeSm)
            color: selected ? studio.accent : studio.textSecondary
            elide: Text.ElideRight
            horizontalAlignment: compact ? Text.AlignHCenter : Text.AlignLeft
        }

        ToolButton {
            id: closeBtn
            visible: closable
            implicitWidth: 18
            implicitHeight: 18
            onClicked: root.closeClicked()
            background: Rectangle {
                radius: studio.radiusSm
                color: closeBtn.hovered ? studio.error : "transparent"
                opacity: closeBtn.hovered ? 0.35 : 0
            }
            contentItem: Label {
                text: "×"
                font.pixelSize: studio.fontSizeSm
                color: studio.textSecondary
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }
    }

    MouseArea {
        id: mouse
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: root.clicked()
    }
}
