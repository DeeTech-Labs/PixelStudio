import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import PixelStudio

pragma Translator: Inspector


InspectorSection {
    id: section
    title: qsTr("Tone")
    hint: qsTr("Adjustments before dithering and rasterization.")

    StudioField { studio: section.studio; labelText: qsTr("Tone preset") }
    StudioCombo {
        id: toneCombo
        Layout.fillWidth: true
        studio: section.studio
        model: [
            qsTr("Custom"),
            qsTr("Icon (high contrast)"),
            qsTr("Photo (natural)")
        ]
        Component.onCompleted: currentIndex = inspectorImage.filters.tonePreset
        onActivated: inspectorImage.filters.setTonePreset(currentIndex)
    }

    StudioCheck {
        studio: section.studio
        text: qsTr("Black background")
        checked: inspectorImage.filters.blackBackground
        onToggled: inspectorImage.filters.setBlackBackground(checked)
    }
    StudioSlider {
        Layout.fillWidth: true
        studio: section.studio
        label: qsTr("Brightness")
        enabled: !inspectorImage.toneLocked
        from: 0; to: 200
        value: inspectorImage.filters.brightness
        onValueCommitted: (v) => inspectorImage.filters.setBrightness(Math.round(v))
    }
    StudioSlider {
        Layout.fillWidth: true
        studio: section.studio
        label: qsTr("Contrast")
        enabled: !inspectorImage.toneLocked
        from: 0; to: 200
        value: inspectorImage.filters.contrast
        onValueCommitted: (v) => inspectorImage.filters.setContrast(Math.round(v))
    }
    StudioCheck {
        studio: section.studio
        text: qsTr("Invert colors")
        checked: inspectorImage.monoOutput ? inspectorImage.transform.invertMono : inspectorImage.filters.filterInvert
        onToggled: {
            if (inspectorImage.monoOutput)
                inspectorImage.transform.setInvertMono(checked)
            else
                inspectorImage.filters.setFilterInvert(checked)
        }
    }

    Connections {
        target: inspectorImage.filters
        function onTonePresetChanged() { toneCombo.currentIndex = inspectorImage.filters.tonePreset }
    }
}
