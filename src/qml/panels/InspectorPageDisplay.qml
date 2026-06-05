import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import PixelStudio

pragma Translator: PixelStudio

Item {
    id: root
    required property var studio
    property bool showFlash: false

    readonly property bool monoEncoding: converter.encodingIsMono1Bit

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
            text: converter.displayWidth + " × " + converter.displayHeight
                  + " · " + converter.encodingModeName
                  + " · " + converter.dataByteCount + " " + qsTr("B")
                  + (converter.codeUseProgmem ? " · PROGMEM" : "")
        }

        StudioSection {
            studio: root.studio
            title: qsTr("Display size")
            hint: qsTr("Width and height define the firmware buffer.")

            StudioField { studio: root.studio; labelText: qsTr("Resolution preset") }
            StudioCombo {
                id: presetCombo
                Layout.fillWidth: true
                studio: root.studio
                readonly property int _localeRev: converter.localizationRevision
                model: _localeRev >= 0 ? converter.displayPresets() : []
                textRole: "name"
                Component.onCompleted: syncPreset()
                onActivated: {
                    const item = model[currentIndex]
                    if (item && item.id)
                        converter.setProfileId(item.id)
                }
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
                    value: converter.displayWidth
                    onValueCommitted: (v) => {
                        if (v !== converter.displayWidth)
                            converter.setDisplayWidth(v)
                    }
                }
                StudioSpin {
                    Layout.fillWidth: true
                    studio: root.studio
                    label: qsTr("Height")
                    from: 8
                    to: 2048
                    value: converter.displayHeight
                    onValueCommitted: (v) => {
                        if (v !== converter.displayHeight)
                            converter.setDisplayHeight(v)
                    }
                }
            }

            StudioButton {
                Layout.fillWidth: true
                studio: root.studio
                text: qsTr("Swap width ↔ height")
                onClicked: converter.swapDisplayDimensions()
            }
        }

        StudioSection {
            studio: root.studio
            title: qsTr("Color encoding")
            hint: qsTr("RGB565, RGB888, ARGB8888, monochrome, grayscale, indexed, YUV and other pixel formats for TFT and OLED.")

            StudioCombo {
                id: encCombo
                Layout.fillWidth: true
                studio: root.studio
                readonly property int _localeRev: converter.localizationRevision
                model: _localeRev >= 0 ? converter.availableEncodingModesForUi() : []
                textRole: "name"
                Component.onCompleted: syncEnc()
                onActivated: {
                    const item = model[currentIndex]
                    if (item && item.mode !== undefined)
                        converter.setEncodingMode(item.mode)
                }
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
                Component.onCompleted: currentIndex = converter.monoLayout
                onActivated: converter.setMonoLayout(currentIndex)
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
                text: converter.arrayName
                onEditingFinished: converter.setArrayName(text)
            }

            StudioCheck {
                studio: root.studio
                text: qsTr("Include header comments")
                checked: converter.codeIncludeComments
                onToggled: converter.setCodeIncludeComments(checked)
            }
            StudioCheck {
                studio: root.studio
                text: qsTr("PROGMEM (Arduino)")
                checked: converter.codeUseProgmem
                onToggled: converter.setCodeUseProgmem(checked)
            }
            StudioCheck {
                studio: root.studio
                text: qsTr("static storage")
                checked: converter.codeStaticStorage
                onToggled: converter.setCodeStaticStorage(checked)
            }
            StudioCheck {
                visible: converter.encodingMode === 4 || converter.encodingMode === 8
                studio: root.studio
                text: qsTr("RGB565 big-endian (SPI)")
                checked: converter.rgb565BigEndian
                onToggled: converter.setRgb565BigEndian(checked)
            }
            StudioCheck {
                studio: root.studio
                text: qsTr("sRGB → linear quantization")
                checked: converter.linearColorSpace
                onToggled: converter.setLinearColorSpace(checked)
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
                onActivated: converter.setCodeDmaAlign(model[currentIndex].value)
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
                enabled: converter.flashReport.length > 0
                onClicked: showFlash = !showFlash
            }
            StudioButton {
                Layout.fillWidth: true
                visible: showFlash
                studio: root.studio
                text: qsTr("Use smallest encoding")
                enabled: converter.flashReport.length > 0
                onClicked: applySmallestEncoding()
            }
            Repeater {
                model: showFlash ? converter.flashReport : []
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

    function syncPreset() {
        for (let i = 0; i < presetCombo.model.length; ++i) {
            if (presetCombo.model[i].id === converter.profileId) {
                presetCombo.currentIndex = i
                return
            }
        }
    }

    function syncDmaAlign() {
        for (let i = 0; i < dmaAlignCombo.model.length; ++i) {
            if (dmaAlignCombo.model[i].value === converter.codeDmaAlign) {
                dmaAlignCombo.currentIndex = i
                return
            }
        }
    }

    function syncEnc() {
        for (let i = 0; i < encCombo.model.length; ++i) {
            if (encCombo.model[i].mode === converter.encodingMode) {
                encCombo.currentIndex = i
                return
            }
        }
    }

    function applySmallestEncoding() {
        for (let i = 0; i < converter.flashReport.length; ++i) {
            const row = converter.flashReport[i]
            if (row.recommended && row.mode !== undefined) {
                converter.setEncodingMode(row.mode)
                return
            }
        }
    }

    Connections {
        target: converter
        function onProfileIdChanged() { syncPreset() }
        function onDisplayWidthChanged() { syncPreset() }
        function onDisplayHeightChanged() { syncPreset() }
        function onColorModeChanged() { syncEnc() }
        function onEncodingModeChanged() { syncEnc() }
        function onCodeDmaAlignChanged() { syncDmaAlign() }
        function onMonoLayoutChanged() {
            monoLayoutCombo.currentIndex = converter.monoLayout
        }
        function onArrayNameChanged() {
            if (arrayField.text !== converter.arrayName)
                arrayField.text = converter.arrayName
        }
    }
}
