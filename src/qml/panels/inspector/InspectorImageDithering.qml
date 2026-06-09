import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import PixelStudio

pragma Translator: Inspector


InspectorSection {
    id: section
    title: qsTr("Dithering")
    hint: inspectorImage.monoOutput
          ? qsTr("Applies when converting to 1-bit monochrome.")
          : qsTr("Available for monochrome encodings only.")
    collapsible: true
    expanded: inspectorImage.monoOutput
    visible: inspectorImage.monoOutput

    StudioCombo {
        id: ditherCombo
        Layout.fillWidth: true
        studio: section.studio
        model: [
            qsTr("None"),
            qsTr("Floyd–Steinberg"),
            qsTr("JJN"),
            qsTr("Bayer")
        ]
        Component.onCompleted: currentIndex = inspectorImage.filters.ditherMode
        onActivated: inspectorImage.filters.setDitherMode(currentIndex)
    }
    StudioSlider {
        Layout.fillWidth: true
        studio: section.studio
        label: qsTr("B&W threshold")
        visible: inspectorImage.monoOutput && inspectorImage.filters.ditherMode === 0
        from: 0
        to: 255
        liveUpdate: false
        value: inspectorImage.output.monoThreshold
        onValueCommitted: (v) => inspectorImage.output.setMonoThreshold(Math.round(v))
    }

    Connections {
        target: inspectorImage.filters
        function onDitherModeChanged() { ditherCombo.currentIndex = inspectorImage.filters.ditherMode }
    }
}
