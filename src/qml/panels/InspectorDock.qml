import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import PixelStudio

pragma Translator: Inspector


Rectangle {
    id: root

    required property var studio
    property int pageIndex: 0
    signal notify(string message)

    readonly property bool stackedHeader: width < 300

    radius: studio.radiusMd
    color: studio.surfaceInset
    border.width: studio.pixelBorderWidth
    border.color: studio.border
    clip: true

    readonly property var pages: {
        const _ = workspace.settings.languageCode + workspace.image.localizationRevision
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

                InspectorTabRow {
                    Layout.fillWidth: true
                    studio: root.studio
                    pages: root.pages
                    pageIndex: root.pageIndex
                    tabFillWidth: true
                    onPageSelected: (pageId) => root.pageIndex = pageId
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

                InspectorTabRow {
                    studio: root.studio
                    pages: root.pages
                    pageIndex: root.pageIndex
                    onPageSelected: (pageId) => root.pageIndex = pageId
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

            StackLayout {
                id: pageStack
                Layout.fillWidth: true
                Layout.preferredHeight: children[root.pageIndex].implicitHeight
                currentIndex: root.pageIndex

                InspectorPageImage {
                    studio: root.studio
                    width: inspectorScroll.availableWidth > 0
                        ? inspectorScroll.availableWidth
                        : root.width
                }

                InspectorPageDisplay {
                    studio: root.studio
                    width: inspectorScroll.availableWidth > 0
                        ? inspectorScroll.availableWidth
                        : root.width
                }

                ExportHub {
                    studio: root.studio
                    width: inspectorScroll.availableWidth > 0
                        ? inspectorScroll.availableWidth
                        : root.width
                    onNotify: (m) => root.notify(m)
                }
            }
        }
    }
}
