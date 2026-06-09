import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import PixelStudio

pragma Translator: Inspector


InspectorSection {
    id: section
    title: qsTr("Advanced color")
    collapsible: true
    expanded: false
    visible: !inspectorImage.grayscaleOutput

    StudioSlider {
        Layout.fillWidth: true
        studio: section.studio
        label: qsTr("Saturation")
        enabled: !inspectorImage.toneLocked
        from: 0; to: 200
        value: inspectorImage.filters.saturation
        onValueCommitted: (v) => inspectorImage.filters.setSaturation(Math.round(v))
    }
    StudioSlider {
        Layout.fillWidth: true
        studio: section.studio
        label: qsTr("Exposure")
        enabled: !inspectorImage.toneLocked
        from: 50; to: 200
        value: inspectorImage.filters.exposure
        valueText: inspectorImage.filters.exposure + "%"
        onValueCommitted: (v) => inspectorImage.filters.setExposure(Math.round(v))
    }
    StudioSlider {
        Layout.fillWidth: true
        studio: section.studio
        label: qsTr("Gamma")
        enabled: !inspectorImage.toneLocked
        from: 50; to: 200
        value: inspectorImage.filters.gamma
        valueText: inspectorImage.filters.gamma + "%"
        onValueCommitted: (v) => inspectorImage.filters.setGamma(Math.round(v))
    }
    StudioSlider {
        Layout.fillWidth: true
        studio: section.studio
        label: qsTr("Blur")
        from: 0; to: 6
        value: inspectorImage.filters.blur
        onValueCommitted: (v) => inspectorImage.filters.setBlur(Math.round(v))
    }
    StudioSlider {
        Layout.fillWidth: true
        studio: section.studio
        label: qsTr("Posterize (RGB)")
        from: 0; to: 30
        value: inspectorImage.filters.posterizeRgb
        onValueCommitted: (v) => inspectorImage.filters.setPosterizeRgb(Math.round(v))
    }
}
