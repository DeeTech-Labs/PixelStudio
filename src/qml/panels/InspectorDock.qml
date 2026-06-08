import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import PixelStudio

pragma Translator: Inspector


Rectangle {
    id: root

    required property var studio
    required property var exportBridge
    property int pageIndex: 0
    signal notify(string message)

    readonly property bool stackedHeader: width < 300

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
            id: headerBar
            Layout.fillWidth: true
            Layout.preferredHeight: root.stackedHeader ? 52 : 30
            color: studio.surface

            ColumnLayout {
                visible: root.stackedHeader
                anchors.fill: parent
                anchors.leftMargin: studio.spacingSm
                anchors.rightMargin: studio.spacingSm
                spacing: studio.spacingXs

                Label {
                    Layout.fillWidth: true
                    text: qsTr("Inspector").toUpperCase()
                    font.family: studio.fontFamilyPixel
                    font.pixelSize: studio.fontSizePixel
                    color: studio.accent
                    elide: Text.ElideRight
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: studio.spacingXs

                    Repeater {
                        model: root.pages
                        delegate: PixelTab {
                            required property var modelData
                            Layout.fillWidth: true
                            Layout.minimumWidth: 0
                            studio: root.studio
                            compact: true
                            label: modelData.title
                            selected: root.pageIndex === modelData.id
                            onClicked: root.pageIndex = modelData.id
                        }
                    }
                }
            }

            RowLayout {
                visible: !root.stackedHeader
                anchors.fill: parent
                anchors.leftMargin: studio.spacingSm
                anchors.rightMargin: studio.spacingSm
                spacing: studio.spacingSm

                Label {
                    Layout.maximumWidth: Math.min(96, headerBar.width * 0.34)
                    text: qsTr("Inspector").toUpperCase()
                    font.family: studio.fontFamilyPixel
                    font.pixelSize: studio.fontSizePixel
                    color: studio.accent
                    elide: Text.ElideRight
                }

                Item { Layout.fillWidth: true; Layout.minimumWidth: studio.spacingSm }

                RowLayout {
                    spacing: studio.spacingXs
                    Repeater {
                        model: root.pages
                        delegate: PixelTab {
                            required property var modelData
                            studio: root.studio
                            compact: true
                            label: modelData.title
                            selected: root.pageIndex === modelData.id
                            onClicked: root.pageIndex = modelData.id
                        }
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
            id: inspectorScroll
            Layout.fillWidth: true
            Layout.fillHeight: true
            studio: root.studio

            Loader {
                id: pageLoader
                Layout.fillWidth: true
                Layout.preferredHeight: item ? item.implicitHeight : 0
                sourceComponent: {
                    switch (root.pageIndex) {
                    case 1:
                        return displayPageComponent
                    case 2:
                        return exportPageComponent
                    default:
                        return imagePageComponent
                    }
                }
            }
        }
    }

    Component {
        id: imagePageComponent
        InspectorPageImage {
            studio: root.studio
            width: inspectorScroll.availableWidth > 0
                ? inspectorScroll.availableWidth
                : root.width
        }
    }

    Component {
        id: displayPageComponent
        InspectorPageDisplay {
            studio: root.studio
            width: inspectorScroll.availableWidth > 0
                ? inspectorScroll.availableWidth
                : root.width
        }
    }

    Component {
        id: exportPageComponent
        InspectorPageExport {
            studio: root.studio
            exportBridge: root.exportBridge
            width: inspectorScroll.availableWidth > 0
                ? inspectorScroll.availableWidth
                : root.width
            onNotify: (m) => root.notify(m)
        }
    }
}
