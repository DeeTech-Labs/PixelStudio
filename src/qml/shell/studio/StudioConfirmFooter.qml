import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import PixelStudio

pragma Translator: Shell


RowLayout {
    id: root

    required property var studio
    required property Dialog dialog

    spacing: studio.spacingMd

    Item { Layout.fillWidth: true }

    StudioButton {
        studio: root.studio
        primary: true
        text: qsTr("Yes")
        onClicked: root.dialog.accept()
    }

    StudioButton {
        studio: root.studio
        text: qsTr("No")
        onClicked: root.dialog.reject()
    }
}
