import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import PixelStudio

pragma Translator: Inspector


Item {
    id: root
    required property var studio

    implicitHeight: column.implicitHeight
    Layout.fillWidth: true
    width: parent ? parent.width : implicitWidth

    ColumnLayout {
        id: column
        width: root.width
        spacing: studio.spacingLg

        InspectorImageInfo { studio: root.studio }
        InspectorImageAdjustments { studio: root.studio }
        InspectorImageAdvanced { studio: root.studio }
        InspectorImageTransform { studio: root.studio }
        InspectorImageTone { studio: root.studio }
        InspectorImageAdvancedColor { studio: root.studio }
        InspectorImageDithering { studio: root.studio }
        InspectorImageEdgesMask { studio: root.studio }

        StudioButton {
            Layout.fillWidth: true
            studio: root.studio
            iconName: "eraser"
            text: qsTr("Reset all filters")
            objectName: "resetAllFiltersButton"
            onClicked: inspectorImage.resetFilters()
        }

        Label {
            Layout.fillWidth: true
            visible: !inspectorImage.hasImage
            wrapMode: Text.WordWrap
            text: qsTr("Load an image to adjust placement and filters.")
            color: studio.textMuted
            font.pixelSize: studio.fontSizeSm
        }
    }
}
