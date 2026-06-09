import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import PixelStudio

pragma Translator: Inspector


RowLayout {
    id: root

    required property var studio
    required property var pages
    required property int pageIndex

    signal pageSelected(int pageId)

    property bool tabFillWidth: false

    spacing: studio.spacingXs

    Repeater {
        model: root.pages
        delegate: PixelTab {
            required property var modelData
            Layout.fillWidth: root.tabFillWidth
            Layout.minimumWidth: root.tabFillWidth ? 0 : implicitWidth
            studio: root.studio
            compact: true
            label: modelData.title
            selected: root.pageIndex === modelData.id
            objectName: "inspectorTab" + modelData.id
            Accessible.name: modelData.title
            onClicked: root.pageSelected(modelData.id)
        }
    }
}
