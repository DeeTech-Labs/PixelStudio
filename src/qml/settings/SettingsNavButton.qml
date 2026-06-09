import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import PixelStudio

pragma Translator: Shell


Rectangle {
    id: root

    required property var studio
    required property string iconName
    required property string label
    property bool active: false

    signal triggered()

    implicitHeight: 40
    width: parent ? parent.width : implicitWidth
    radius: studio.radiusMd
    color: active ? studio.railActive : (hoverMa.containsMouse ? studio.railHover : "transparent")
    border.width: active ? 1 : 0
    border.color: active ? studio.accentMuted : "transparent"

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: studio.spacingMd
        anchors.rightMargin: studio.spacingMd
        spacing: studio.spacingSm

        Rectangle {
            Layout.preferredWidth: 3
            Layout.preferredHeight: 18
            radius: 1
            visible: root.active
            color: studio.accent
        }

        StudioIcon {
            name: root.iconName
            iconSize: 14
            tint: root.active ? studio.accent : studio.textMuted
        }

        Label {
            Layout.fillWidth: true
            text: root.label
            font.family: studio.fontFamily
            font.pixelSize: studio.fontSizeSm
            font.weight: root.active ? Font.DemiBold : Font.Normal
            color: root.active ? studio.text : studio.textSecondary
            elide: Text.ElideRight
        }
    }

    MouseArea {
        id: hoverMa
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: root.triggered()
    }
}
