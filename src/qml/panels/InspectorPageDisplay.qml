import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import PixelStudio

pragma Translator: Inspector


Item {
    id: root
    required property var studio
    property bool showFlash: false

    readonly property bool monoEncoding: inspectorDisplay.monoEncoding

    implicitHeight: column.implicitHeight
    Layout.fillWidth: true
    width: parent ? parent.width : implicitWidth

    ColumnLayout {
        id: column
        width: root.width
        spacing: studio.spacingLg

        Label {
            Layout.fillWidth: true
            font.family: studio.fontFamilyMono
            font.pixelSize: studio.fontSizeSm
            color: studio.textMuted
            text: workspace.output.displayWidth + " × " + workspace.output.displayHeight
                  + " · " + workspace.output.encodingModeName
                  + " · " + workspace.output.dataByteCount + " " + qsTr("B")
                  + (workspace.code.codeUseProgmem ? " · PROGMEM" : "")
        }

        StudioSection {
            studio: root.studio
            title: qsTr("Display size")
            hint: qsTr("Width and height define the firmware buffer.")

            StudioField { studio: root.studio; labelText: qsTr("Resolution preset") }
            StudioBoundCombo {
                id: presetCombo
                Layout.fillWidth: true
                studio: root.studio
                boundModel: workspace.output.displayPresetsModel
                textRole: "name"
                valueRole: "id"
                boundValue: workspace.output.profileId
                onValueSelected: (id) => workspace.output.setProfileId(id)
            }

            RowLayout {
                Layout.fillWidth: true
                StudioSpin {
                    Layout.fillWidth: true
                    studio: root.studio
                    label: qsTr("Width")
                    from: 8
                    to: 2048
                    stepSize: 1
                    value: workspace.output.displayWidth
                    onValueCommitted: (v) => {
                        if (v !== workspace.output.displayWidth)
                            workspace.output.setDisplayWidth(v)
                    }
                }
                StudioSpin {
                    Layout.fillWidth: true
                    studio: root.studio
                    label: qsTr("Height")
                    from: 8
                    to: 2048
                    value: workspace.output.displayHeight
                    onValueCommitted: (v) => {
                        if (v !== workspace.output.displayHeight)
                            workspace.output.setDisplayHeight(v)
                    }
                }
            }

            StudioButton {
                Layout.fillWidth: true
                studio: root.studio
                text: qsTr("Swap width ↔ height")
                onClicked: workspace.output.swapDisplayDimensions()
            }
        }

        StudioSection {
            studio: root.studio
            title: qsTr("Color encoding")
            hint: qsTr("RGB565, RGB888, ARGB8888, monochrome, grayscale, indexed, YUV and other pixel formats for TFT and OLED.")

            StudioBoundCombo {
                id: encCombo
                Layout.fillWidth: true
                studio: root.studio
                boundModel: workspace.output.encodingModesModel
                textRole: "name"
                valueRole: "mode"
                boundValue: workspace.output.encodingMode
                onValueSelected: (mode) => workspace.output.setEncodingMode(mode)
            }
        }

        StudioSection {
            visible: root.monoEncoding
            studio: root.studio
            title: qsTr("Monochrome memory layout")
            hint: qsTr("How 1-bit pixels are packed in RAM (row or page buffer).")

            StudioCombo {
                id: monoLayoutCombo
                Layout.fillWidth: true
                studio: root.studio
                model: [
                    qsTr("Row-packed"),
                    qsTr("Vertical page buffer"),
                    qsTr("Vertical column")
                ]
                Component.onCompleted: currentIndex = workspace.output.monoLayout
                onActivated: workspace.output.setMonoLayout(currentIndex)
            }
        }

        StudioSection {
            studio: root.studio
            title: qsTr("Generated array")
            hint: qsTr("Symbol name and storage modifiers for C/C++ output.")

            StudioField { studio: root.studio; labelText: qsTr("Array name") }
            StudioTextField {
                id: arrayField
                Layout.fillWidth: true
                studio: root.studio
                text: workspace.code.arrayName
                onEditingFinished: workspace.code.setArrayName(text)
            }

            StudioCheck {
                studio: root.studio
                text: qsTr("Include header comments")
                checked: workspace.code.codeIncludeComments
                onToggled: workspace.code.setCodeIncludeComments(checked)
            }
            StudioCheck {
                studio: root.studio
                text: qsTr("PROGMEM (Arduino)")
                checked: workspace.code.codeUseProgmem
                onToggled: workspace.code.setCodeUseProgmem(checked)
            }
            StudioCheck {
                studio: root.studio
                text: qsTr("static storage")
                checked: workspace.code.codeStaticStorage
                onToggled: workspace.code.setCodeStaticStorage(checked)
            }
            StudioCheck {
                visible: workspace.output.encodingMode === 4 || workspace.output.encodingMode === 8
                studio: root.studio
                text: qsTr("RGB565 big-endian (SPI)")
                checked: workspace.code.rgb565BigEndian
                onToggled: workspace.code.setRgb565BigEndian(checked)
            }
            StudioCheck {
                studio: root.studio
                text: qsTr("sRGB → linear quantization")
                checked: workspace.output.linearColorSpace
                onToggled: workspace.output.setLinearColorSpace(checked)
            }
            StudioField { studio: root.studio; labelText: qsTr("DMA buffer alignment") }
            StudioCombo {
                id: dmaAlignCombo
                Layout.fillWidth: true
                studio: root.studio
                model: [
                    { label: qsTr("None"), value: 0 },
                    { label: qsTr("4 bytes"), value: 4 },
                    { label: qsTr("8 bytes"), value: 8 }
                ]
                textRole: "label"
                Component.onCompleted: syncDmaAlign()
                onActivated: workspace.code.setCodeDmaAlign(model[currentIndex].value)
            }
        }

        StudioSection {
            studio: root.studio
            title: qsTr("Flash size")
            hint: qsTr("Compare encodings by size in flash memory.")

            StudioButton {
                Layout.fillWidth: true
                studio: root.studio
                text: showFlash ? qsTr("Hide flash size advisor") : qsTr("Flash size advisor")
                enabled: workspace.image.flashReport.length > 0
                onClicked: showFlash = !showFlash
            }
            StudioButton {
                Layout.fillWidth: true
                visible: showFlash
                studio: root.studio
                text: qsTr("Use smallest encoding")
                enabled: workspace.image.flashReport.length > 0
                onClicked: inspectorDisplay.applySmallestEncoding()
            }
            Repeater {
                model: showFlash ? workspace.image.flashReport : []
                delegate: Label {
                    required property var modelData
                    Layout.fillWidth: true
                    font.family: root.studio.fontFamilyMono
                    font.pixelSize: root.studio.fontSizeSm
                    color: modelData.current ? root.studio.text
                          : modelData.recommended ? root.studio.accent
                          : root.studio.textMuted
                    text: modelData.name
                          + (modelData.current ? qsTr(" · current") : "")
                          + "  " + modelData.bytes + " " + qsTr("B")
                          + (modelData.recommended ? qsTr(" · best") : "")
                }
            }
        }
    }

    function syncDmaAlign() {
        for (let i = 0; i < dmaAlignCombo.model.length; ++i) {
            if (dmaAlignCombo.model[i].value === workspace.code.codeDmaAlign) {
                dmaAlignCombo.currentIndex = i
                return
            }
        }
    }

    Connections {
        target: workspace.output
        function onMonoLayoutChanged() {
            monoLayoutCombo.currentIndex = workspace.output.monoLayout
        }
    }

    Connections {
        target: workspace.code
        function onCodeDmaAlignChanged() { syncDmaAlign() }
        function onArrayNameChanged() {
            if (arrayField.text !== workspace.code.arrayName)
                arrayField.text = workspace.code.arrayName
        }
    }
}
