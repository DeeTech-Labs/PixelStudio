import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

pragma Translator: Controls


Item {
    id: root

    required property var studio
    default property alias content: section.content
    property string title: ""
    property string hint: ""
    property bool collapsible: false
    property bool expanded: true

    implicitHeight: root.visible ? section.implicitHeight : 0
    Layout.fillWidth: true
    width: parent ? parent.width : implicitWidth

    StudioSection {
        id: section
        anchors.fill: parent
        visible: root.visible
        studio: root.studio
        title: root.title
        hint: root.hint
        collapsible: root.collapsible
        expanded: root.expanded
    }
}
