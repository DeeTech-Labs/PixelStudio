import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import PixelStudio

pragma Translator: Inspector


InspectorSection {
    id: section

    title: qsTr("Fit and placement")
    hint: qsTr("How the image is scaled and positioned in the buffer.")

    StudioCombo {
        id: scaleCombo
        Layout.fillWidth: true
        studio: section.studio
        model: [
            qsTr("Fit (letterbox)"),
            qsTr("Stretch to fill"),
            qsTr("Crop center")
        ]
        Component.onCompleted: currentIndex = inspectorImage.transform.scaleMode
        onActivated: inspectorImage.transform.setScaleMode(currentIndex)
    }

    StudioSegmented {
        id: rotationSeg
        Layout.fillWidth: true
        studio: section.studio
        segments: [
            { label: qsTr("0"), value: 0 },
            { label: qsTr("90"), value: 90 },
            { label: qsTr("180"), value: 180 },
            { label: qsTr("270"), value: 270 }
        ]
        selectedValue: inspectorImage.transform.rotation
        onSegmentActivated: (v) => inspectorImage.transform.setRotation(v)
    }

    ColumnLayout {
        Layout.fillWidth: true
        spacing: section.studio.spacingXs
        StudioCheck {
            Layout.fillWidth: true
            studio: section.studio
            text: qsTr("Flip horizontal")
            checked: inspectorImage.transform.flipHorizontal
            onToggled: inspectorImage.transform.setFlipHorizontal(checked)
        }
        StudioCheck {
            Layout.fillWidth: true
            studio: section.studio
            text: qsTr("Flip vertical")
            checked: inspectorImage.transform.flipVertical
            onToggled: inspectorImage.transform.setFlipVertical(checked)
        }
    }

    RowLayout {
        Layout.fillWidth: true
        StudioSpin {
            Layout.fillWidth: true
            studio: section.studio
            label: "X"
            from: -512
            to: 512
            value: inspectorImage.transform.offsetX
            onValueCommitted: (v) => {
                if (v !== inspectorImage.transform.offsetX)
                    inspectorImage.transform.setOffsetX(v)
            }
        }
        StudioSpin {
            Layout.fillWidth: true
            studio: section.studio
            label: "Y"
            from: -512
            to: 512
            value: inspectorImage.transform.offsetY
            onValueCommitted: (v) => {
                if (v !== inspectorImage.transform.offsetY)
                    inspectorImage.transform.setOffsetY(v)
            }
        }
    }

    StudioButton {
        Layout.fillWidth: true
        studio: section.studio
        text: qsTr("Center on display")
        enabled: inspectorImage.hasImage
        onClicked: inspectorImage.transform.centerOffsetOnDisplay()
    }
    StudioButton {
        Layout.fillWidth: true
        studio: section.studio
        iconName: "rotate-cw"
        text: qsTr("Reset transform")
        enabled: inspectorImage.hasImage
        onClicked: inspectorImage.transform.resetTransform()
    }

    Connections {
        target: inspectorImage.transform
        function onRotationChanged() { rotationSeg.selectedValue = inspectorImage.transform.rotation }
        function onScaleModeChanged() { scaleCombo.currentIndex = inspectorImage.transform.scaleMode }
    }
}
