import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import PixelStudio

pragma Translator: Inspector


InspectorSection {
    id: section
    title: qsTr("Adjustments")
    hint: qsTr("Contrast, dithering and color reduction before rasterization.")

    StudioSlider {
        Layout.fillWidth: true
        studio: section.studio
        label: qsTr("Contrast")
        enabled: !inspectorImage.toneLocked
        from: 0; to: 200
        value: inspectorImage.filters.contrast
        valueText: (inspectorImage.filters.contrast / 100).toFixed(2)
        onValueCommitted: (v) => inspectorImage.filters.setContrast(Math.round(v))
    }
    StudioCheck {
        studio: section.studio
        text: qsTr("Dither")
        checked: inspectorImage.filters.dithering
        onToggled: inspectorImage.filters.setDithering(checked)
    }
    StudioBoundCombo {
        Layout.fillWidth: true
        studio: section.studio
        boundModel: inspectorImage.encodingModesModel
        textRole: "name"
        valueRole: "mode"
        boundValue: inspectorImage.output.encodingMode
        onValueSelected: (mode) => inspectorImage.output.setEncodingMode(mode)
    }
    StudioSlider {
        Layout.fillWidth: true
        studio: section.studio
        label: qsTr("Max colors")
        from: 0; to: 30
        value: inspectorImage.filters.posterizeRgb
        onValueCommitted: (v) => inspectorImage.filters.setPosterizeRgb(Math.round(v))
    }
}
