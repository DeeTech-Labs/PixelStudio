import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import PixelStudio

pragma Translator: PixelStudio

Item {
    id: root

    required property var studio
    required property var window

    signal exitAccepted()
    signal resetSessionAccepted()
    signal closeTabAccepted()
    signal closeTabRejected()

    property string pendingCloseTabTitle: ""
    property int bodyWidth: 360

    StudioDialog {
        id: exitConfirmDialog
        studio: root.studio
        title: qsTr("Exit PixelStudio?")
        modal: true
        anchors.centerIn: parent
        standardButtons: Dialog.Yes | Dialog.No
        width: bodyWidth + 2 * padding
        onAccepted: root.exitAccepted()
        contentItem: Label {
            width: bodyWidth
            wrapMode: Text.WordWrap
            text: qsTr("Close the application? Unsaved exported files or project changes may be lost.")
            font.family: studio.fontFamily
            color: studio.text
        }
        function openDialog() { open() }
    }

    StudioDialog {
        id: resetSessionConfirmDialog
        studio: root.studio
        title: qsTr("Reset session?")
        modal: true
        anchors.centerIn: parent
        standardButtons: Dialog.Yes | Dialog.No
        width: bodyWidth + 2 * padding
        onAccepted: root.resetSessionAccepted()
        contentItem: Label {
            width: bodyWidth
            wrapMode: Text.WordWrap
            text: qsTr("Clears inspector settings, recent lists, and watch folders. Projects on disk are not deleted.")
            font.family: studio.fontFamily
            color: studio.text
        }
        function openDialog() { open() }
    }

    StudioDialog {
        id: closeTabConfirmDialog
        studio: root.studio
        title: qsTr("Close tab?")
        modal: true
        anchors.centerIn: parent
        standardButtons: Dialog.Yes | Dialog.No
        width: bodyWidth + 2 * padding
        onAccepted: root.closeTabAccepted()
        onRejected: root.closeTabRejected()
        contentItem: ColumnLayout {
            width: bodyWidth
            spacing: studio.spacingMd
            Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                text: qsTr("Close “%1”? Unsaved changes in this tab may be lost.")
                    .arg(root.pendingCloseTabTitle)
                font.family: studio.fontFamily
                color: studio.text
            }
            StudioCheck {
                id: closeTabDontAsk
                studio: root.studio
                text: qsTr("Don't ask again")
            }
        }
        function openDialog() { open() }
        function dontAskAgain() { return closeTabDontAsk.checked }
    }
}
