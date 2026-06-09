import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import PixelStudio

pragma Translator: Inspector


InspectorSection {
    id: section
    title: qsTr("Edges and mask")
    collapsible: true
    expanded: false

    StudioCheck {
        studio: section.studio
        text: qsTr("Sharpen")
        checked: inspectorImage.filters.sharpen
        onToggled: inspectorImage.filters.setSharpen(checked)
    }
    StudioSlider {
        Layout.fillWidth: true
        studio: section.studio
        label: qsTr("Sobel edges")
        from: 0; to: 100
        value: inspectorImage.filters.sobelEdges
        onValueCommitted: (v) => inspectorImage.filters.setSobelEdges(Math.round(v))
    }
    StudioSlider {
        Layout.fillWidth: true
        studio: section.studio
        label: qsTr("Posterize (gray)")
        from: 0; to: 30
        value: inspectorImage.filters.posterizeGray
        onValueCommitted: (v) => inspectorImage.filters.setPosterizeGray(Math.round(v))
    }
    StudioField { studio: section.studio; labelText: qsTr("Contours") }
    StudioCombo {
        id: contourCombo
        Layout.fillWidth: true
        studio: section.studio
        model: [
            qsTr("None"),
            qsTr("4-connected"),
            qsTr("8-connected")
        ]
        Component.onCompleted: currentIndex = inspectorImage.filters.contourMode
        onActivated: inspectorImage.filters.setContourMode(currentIndex)
    }

    StudioCheck {
        id: maskOn
        studio: section.studio
        text: qsTr("Enable color mask")
        checked: inspectorImage.filters.colorMaskEnabled
        onToggled: inspectorImage.filters.setColorMaskEnabled(checked)
    }
    StudioTextField {
        Layout.fillWidth: true
        visible: maskOn.checked
        studio: section.studio
        text: inspectorImage.filters.maskColor
        placeholderText: "#RRGGBB"
        onEditingFinished: inspectorImage.filters.setMaskColor(text)
    }
    StudioSlider {
        Layout.fillWidth: true
        visible: maskOn.checked
        studio: section.studio
        label: qsTr("Tolerance")
        from: 0; to: 255
        value: inspectorImage.filters.maskTolerance
        onValueCommitted: (v) => inspectorImage.filters.setMaskTolerance(Math.round(v))
    }
    StudioSlider {
        Layout.fillWidth: true
        visible: maskOn.checked
        studio: section.studio
        label: qsTr("Amplify")
        from: 1; to: 10
        value: inspectorImage.filters.maskAmplify
        onValueCommitted: (v) => inspectorImage.filters.setMaskAmplify(Math.round(v))
    }

    Connections {
        target: inspectorImage.filters
        function onContourModeChanged() { contourCombo.currentIndex = inspectorImage.filters.contourMode }
    }
}
