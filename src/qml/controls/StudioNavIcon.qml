import QtQuick
import QtQuick.Controls

pragma Translator: PixelStudio

Item {
    id: root
    required property var studio
    required property string iconName
    required property string label
    required property bool selected

    signal clicked()

    implicitWidth: parent ? parent.width : studio.inspectorNavWidth
    implicitHeight: 40

    Rectangle {
        anchors.fill: parent
        anchors.margins: 4
        radius: studio.radiusMd
        color: root.selected
            ? studio.railActive
            : (navMa.containsMouse ? studio.railHover : "transparent")
    }

    StudioIcon {
        anchors.centerIn: parent
        name: root.iconName
        iconSize: 18
        tint: root.selected ? studio.accent : studio.textSecondary
    }

    MouseArea {
        id: navMa
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: root.clicked()
    }

    ToolTip {
        visible: navMa.containsMouse
        delay: 300
        text: root.label
    }
}
