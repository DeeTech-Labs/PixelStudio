import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import PixelStudio

pragma Translator: Inspector


InspectorSection {
    id: section
    title: qsTr("Advanced")
    collapsible: true
    expanded: inspectorImage.monoOutput

    StudioCheck {
        studio: section.studio
        text: qsTr("Serpentine")
        checked: inspectorImage.filters.ditherMode === 1
        enabled: false
    }
    StudioSlider {
        Layout.fillWidth: true
        studio: section.studio
        label: qsTr("Edge threshold")
        from: 0; to: 100
        value: inspectorImage.filters.sobelEdges
        valueText: inspectorImage.filters.sobelEdges + "%"
        onValueCommitted: (v) => inspectorImage.filters.setSobelEdges(Math.round(v))
    }
}
