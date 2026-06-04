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

    color: studio.surface
    border.width: 1
    border.color: studio.divider

    readonly property var pages: {
        const _ = appSettings.languageCode + converter.localizationRevision
        return [
            { id: 0, icon: "layout-grid", title: qsTr("Display") },
            { id: 1, icon: "sliders-horizontal", title: qsTr("Image") },
            { id: 2, icon: "download", title: qsTr("Export") }
        ]
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.preferredWidth: studio.inspectorNavWidth
            Layout.fillHeight: true
            color: studio.surfaceInset

            ColumnLayout {
                anchors.fill: parent
                anchors.topMargin: studio.spacingSm
                spacing: 2

                Repeater {
                    model: root.pages
                    delegate: StudioNavIcon {
                        required property var modelData
                        required property int index
                        Layout.fillWidth: true
                        studio: root.studio
                        iconName: modelData.icon
                        readonly property int _navLang: appSettings.languageCode.length
                            + converter.localizationRevision
                        label: {
                            const _ = _navLang
                            switch (modelData.id) {
                            case 0: return qsTr("Display")
                            case 1: return qsTr("Image")
                            case 2: return qsTr("Export")
                            }
                            return ""
                        }
                        selected: root.pageIndex === modelData.id
                        onClicked: root.pageIndex = modelData.id
                    }
                }

                Item { Layout.fillWidth: true; Layout.fillHeight: true }
            }
        }

        StudioScroll {
            Layout.fillWidth: true
            Layout.fillHeight: true
            studio: root.studio

            InspectorPageDisplay {
                Layout.fillWidth: true
                visible: root.pageIndex === 0
                studio: root.studio
            }
            InspectorPageImage {
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
