import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import PixelStudio

pragma Translator: PixelStudio

Rectangle {
    id: root

    required property var studio
    required property var exportBridge
    property int pageIndex: 0
    signal notify(string message)

    radius: studio.radiusMd
    color: studio.surfaceInset
    border.width: studio.pixelBorderWidth
    border.color: studio.border
    clip: true

    readonly property var pages: {
        const _ = appSettings.languageCode + converter.localizationRevision
        return [
            { id: 0, title: qsTr("Image") },
            { id: 1, title: qsTr("Display") },
            { id: 2, title: qsTr("Export") }
        ]
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 30
            color: studio.surface

            Label {
                anchors.left: parent.left
                anchors.leftMargin: studio.spacingSm
                anchors.verticalCenter: parent.verticalCenter
                text: qsTr("Inspector").toUpperCase()
                font.family: studio.fontFamilyPixel
                font.pixelSize: studio.fontSizePixel
                color: studio.accent
            }

            Row {
                anchors.right: parent.right
                anchors.rightMargin: studio.spacingSm
                anchors.verticalCenter: parent.verticalCenter
                spacing: studio.spacingXs

                Repeater {
                    model: root.pages
                    delegate: PixelTab {
                        required property var modelData
                        studio: root.studio
                        label: modelData.title
                        selected: root.pageIndex === modelData.id
                        onClicked: root.pageIndex = modelData.id
                    }
                }
            }

            Rectangle {
                anchors.bottom: parent.bottom
                width: parent.width
                height: 1
                color: studio.border
            }
        }

        StudioScroll {
            Layout.fillWidth: true
            Layout.fillHeight: true
            studio: root.studio

            InspectorPageImage {
                Layout.fillWidth: true
                visible: root.pageIndex === 0
                studio: root.studio
            }
            InspectorPageDisplay {
                Layout.fillWidth: true
                visible: root.pageIndex === 1
                studio: root.studio
            }
            InspectorPageExport {
                Layout.fillWidth: true
                visible: root.pageIndex === 2
                studio: root.studio
                exportBridge: root.exportBridge
                onNotify: (m) => root.notify(m)
            }
        }
    }
}
