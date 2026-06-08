import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

pragma Translator: Controls


ScrollView {
    id: root
    required property var studio

    clip: true
    leftPadding: studio.inspectorPadding
    rightPadding: studio.inspectorPadding
    topPadding: studio.spacingMd
    bottomPadding: studio.spacingLg
    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

    ScrollBar.vertical: ScrollBar {
        implicitWidth: studio.scrollBarWidth
        policy: ScrollBar.AsNeeded
        contentItem: Rectangle {
            implicitWidth: studio.scrollBarWidth
            radius: studio.scrollBarRadius
            color: studio.border
            opacity: active || hovered ? 0.9 : 0.35
        }
    }

    default property alias content: contentItem.data

    ColumnLayout {
        id: contentItem
        width: root.availableWidth > 0 ? root.availableWidth : implicitWidth
        spacing: studio.spacingLg
    }
}
