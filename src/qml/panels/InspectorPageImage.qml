import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import PixelStudio

pragma Translator: Inspector


Item {
    id: root
    required property var studio

    readonly property bool monoOutput: displayOutput.encodingIsMono1Bit
    readonly property bool grayscaleOutput: displayOutput.encodingIsGrayscale
    readonly property bool toneLocked: imageFilters.tonePreset !== 0

    implicitHeight: column.implicitHeight
    Layout.fillWidth: true
    width: parent ? parent.width : implicitWidth

    ColumnLayout {
        id: column
        width: root.width
        spacing: studio.spacingLg

        StudioSection {
            studio: root.studio
            title: qsTr("Image info")
            hint: qsTr("Metadata for the active source image.")

            GridLayout {
                Layout.fillWidth: true
                columns: 2
                columnSpacing: studio.spacingMd
                rowSpacing: studio.spacingXs

                Label { text: qsTr("Size"); font.pixelSize: studio.fontSizeXs; color: studio.textMuted }
                Label {
                    text: converter.hasImage
                        ? converter.sourceWidth + " × " + converter.sourceHeight
                        : "—"
                    font.family: studio.fontFamilyMono
                    font.pixelSize: studio.fontSizeXs
                    color: studio.text
                }
                Label { text: qsTr("Colors"); font.pixelSize: studio.fontSizeXs; color: studio.textMuted }
                Label {
                    text: converter.previewColorCount > 0 ? converter.previewColorCount : "—"
                    font.family: studio.fontFamilyMono
                    font.pixelSize: studio.fontSizeXs
                    color: studio.text
                }
                Label { text: qsTr("BPP"); font.pixelSize: studio.fontSizeXs; color: studio.textMuted }
                Label {
                    text: converter.hasPreview ? displayOutput.encodingModeName : "—"
                    font.family: studio.fontFamilyMono
                    font.pixelSize: studio.fontSizeXs
                    color: studio.text
                }
                Label { text: qsTr("Format"); font.pixelSize: studio.fontSizeXs; color: studio.textMuted }
                Label {
                    text: converter.imageFormatName.length > 0 ? converter.imageFormatName : "—"
                    font.family: studio.fontFamilyMono
                    font.pixelSize: studio.fontSizeXs
                    color: studio.text
                }
            }
        }

        StudioSection {
            studio: root.studio
            title: qsTr("Adjustments")
            hint: qsTr("Contrast, dithering and color reduction before rasterization.")

            StudioSlider {
                Layout.fillWidth: true
                studio: root.studio
                label: qsTr("Contrast")
                enabled: !toneLocked
                from: 0; to: 200
                value: imageFilters.contrast
                valueText: (imageFilters.contrast / 100).toFixed(2)
                onValueCommitted: (v) => imageFilters.setContrast(Math.round(v))
            }
            StudioCheck {
                studio: root.studio
                text: qsTr("Dither")
                checked: imageFilters.dithering
                onToggled: imageFilters.setDithering(checked)
            }
            StudioCombo {
                id: encQuickCombo
                Layout.fillWidth: true
                studio: root.studio
                readonly property int _localeRev: converter.localizationRevision
                model: _localeRev >= 0 ? displayOutput.availableEncodingModesForUi() : []
                textRole: "name"
                Component.onCompleted: syncEncQuick()
                onActivated: {
                    const item = model[currentIndex]
                    if (item && item.mode !== undefined)
                        displayOutput.setEncodingMode(item.mode)
                }
                function syncEncQuick() {
                    for (let i = 0; i < model.length; ++i) {
                        if (model[i].mode === displayOutput.encodingMode) {
                            currentIndex = i
                            return
                        }
                    }
                }
            }
            StudioSlider {
                Layout.fillWidth: true
                studio: root.studio
                label: qsTr("Max colors")
                from: 0; to: 30
                value: imageFilters.posterizeRgb
                onValueCommitted: (v) => imageFilters.setPosterizeRgb(Math.round(v))
            }
        }

        StudioSection {
            studio: root.studio
            title: qsTr("Advanced")
            collapsible: true
            expanded: root.monoOutput

            StudioCombo {
                id: ditherComboTop
                Layout.fillWidth: true
                studio: root.studio
                model: [
                    qsTr("None"),
                    qsTr("Floyd–Steinberg"),
                    qsTr("JJN"),
                    qsTr("Bayer")
                ]
                Component.onCompleted: currentIndex = imageFilters.ditherMode
                onActivated: imageFilters.setDitherMode(currentIndex)
            }
            StudioCheck {
                studio: root.studio
                text: qsTr("Serpentine")
                checked: imageFilters.ditherMode === 1
                enabled: false
            }
            StudioSlider {
                Layout.fillWidth: true
                studio: root.studio
                label: qsTr("Edge threshold")
                from: 0; to: 100
                value: imageFilters.sobelEdges
                valueText: imageFilters.sobelEdges + "%"
                onValueCommitted: (v) => imageFilters.setSobelEdges(Math.round(v))
            }
        }

        StudioSection {
            studio: root.studio
            title: qsTr("Fit and placement")
            hint: qsTr("How the image is scaled and positioned in the buffer.")

            StudioCombo {
                id: scaleCombo
                Layout.fillWidth: true
                studio: root.studio
                model: [
                    qsTr("Fit (letterbox)"),
                    qsTr("Stretch to fill"),
                    qsTr("Crop center")
                ]
                Component.onCompleted: currentIndex = imageTransform.scaleMode
                onActivated: imageTransform.setScaleMode(currentIndex)
            }

            StudioSegmented {
                id: rotationSeg
                Layout.fillWidth: true
                studio: root.studio
                segments: [
                    { label: qsTr("0"), value: 0 },
                    { label: qsTr("90"), value: 90 },
                    { label: qsTr("180"), value: 180 },
                    { label: qsTr("270"), value: 270 }
                ]
                selectedValue: imageTransform.rotation
                onSegmentActivated: (v) => imageTransform.setRotation(v)
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: studio.spacingXs
                StudioCheck {
                    Layout.fillWidth: true
                    studio: root.studio
                    text: qsTr("Flip horizontal")
                    checked: imageTransform.flipHorizontal
                    onToggled: imageTransform.setFlipHorizontal(checked)
                }
                StudioCheck {
                    Layout.fillWidth: true
                    studio: root.studio
                    text: qsTr("Flip vertical")
                    checked: imageTransform.flipVertical
                    onToggled: imageTransform.setFlipVertical(checked)
                }
            }

            RowLayout {
                Layout.fillWidth: true
                StudioSpin {
                    Layout.fillWidth: true
                    studio: root.studio
                    label: "X"
                    from: -512
                    to: 512
                    value: imageTransform.offsetX
                    onValueCommitted: (v) => {
                        if (v !== imageTransform.offsetX)
                            imageTransform.setOffsetX(v)
                    }
                }
                StudioSpin {
                    Layout.fillWidth: true
                    studio: root.studio
                    label: "Y"
                    from: -512
                    to: 512
                    value: imageTransform.offsetY
                    onValueCommitted: (v) => {
                        if (v !== imageTransform.offsetY)
                            imageTransform.setOffsetY(v)
                    }
                }
            }

            StudioButton {
                Layout.fillWidth: true
                studio: root.studio
                text: qsTr("Center on display")
                enabled: converter.hasImage
                onClicked: imageTransform.centerOffsetOnDisplay()
            }
            StudioButton {
                Layout.fillWidth: true
                studio: root.studio
                iconName: "rotate-cw"
                text: qsTr("Reset transform")
                enabled: converter.hasImage
                onClicked: imageTransform.resetTransform()
            }
        }

        StudioSection {
            studio: root.studio
            title: qsTr("Tone")
            hint: qsTr("Adjustments before dithering and rasterization.")

            StudioField { studio: root.studio; labelText: qsTr("Tone preset") }
            StudioCombo {
                id: toneCombo
                Layout.fillWidth: true
                studio: root.studio
                model: [
                    qsTr("Custom"),
                    qsTr("Icon (high contrast)"),
                    qsTr("Photo (natural)")
                ]
                Component.onCompleted: currentIndex = imageFilters.tonePreset
                onActivated: imageFilters.setTonePreset(currentIndex)
            }

            StudioCheck {
                studio: root.studio
                text: qsTr("Black background")
                checked: imageFilters.blackBackground
                onToggled: imageFilters.setBlackBackground(checked)
            }
            StudioSlider {
                Layout.fillWidth: true
                studio: root.studio
                label: qsTr("Brightness")
                enabled: !toneLocked
                from: 0; to: 200
                value: imageFilters.brightness
                onValueCommitted: (v) => imageFilters.setBrightness(Math.round(v))
            }
            StudioSlider {
                Layout.fillWidth: true
                studio: root.studio
                label: qsTr("Contrast")
                enabled: !toneLocked
                from: 0; to: 200
                value: imageFilters.contrast
                onValueCommitted: (v) => imageFilters.setContrast(Math.round(v))
            }
            StudioCheck {
                studio: root.studio
                text: qsTr("Invert colors")
                checked: root.monoOutput ? imageTransform.invertMono : imageFilters.filterInvert
                onToggled: {
                    if (root.monoOutput)
                        imageTransform.setInvertMono(checked)
                    else
                        imageFilters.setFilterInvert(checked)
                }
            }
        }

        StudioSection {
            studio: root.studio
            title: qsTr("Advanced color")
            collapsible: true
            expanded: false
            visible: !root.grayscaleOutput

            StudioSlider {
                Layout.fillWidth: true
                studio: root.studio
                label: qsTr("Saturation")
                enabled: !toneLocked
                from: 0; to: 200
                value: imageFilters.saturation
                onValueCommitted: (v) => imageFilters.setSaturation(Math.round(v))
            }
            StudioSlider {
                Layout.fillWidth: true
                studio: root.studio
                label: qsTr("Exposure")
                enabled: !toneLocked
                from: 50; to: 200
                value: imageFilters.exposure
                valueText: imageFilters.exposure + "%"
                onValueCommitted: (v) => imageFilters.setExposure(Math.round(v))
            }
            StudioSlider {
                Layout.fillWidth: true
                studio: root.studio
                label: qsTr("Gamma")
                enabled: !toneLocked
                from: 50; to: 200
                value: imageFilters.gamma
                valueText: imageFilters.gamma + "%"
                onValueCommitted: (v) => imageFilters.setGamma(Math.round(v))
            }
            StudioSlider {
                Layout.fillWidth: true
                studio: root.studio
                label: qsTr("Blur")
                from: 0; to: 6
                value: imageFilters.blur
                onValueCommitted: (v) => imageFilters.setBlur(Math.round(v))
            }
            StudioSlider {
                Layout.fillWidth: true
                studio: root.studio
                label: qsTr("Posterize (RGB)")
                from: 0; to: 30
                value: imageFilters.posterizeRgb
                onValueCommitted: (v) => imageFilters.setPosterizeRgb(Math.round(v))
            }
        }

        StudioSection {
            studio: root.studio
            title: qsTr("Dithering")
            hint: root.monoOutput
                  ? qsTr("Applies when converting to 1-bit monochrome.")
                  : qsTr("Available for monochrome encodings only.")
            collapsible: true
            expanded: root.monoOutput
            visible: root.monoOutput

            StudioCombo {
                id: ditherCombo
                Layout.fillWidth: true
                studio: root.studio
                model: [
                    qsTr("None"),
                    qsTr("Floyd–Steinberg"),
                    qsTr("JJN"),
                    qsTr("Bayer")
                ]
                Component.onCompleted: currentIndex = imageFilters.ditherMode
                onActivated: imageFilters.setDitherMode(currentIndex)
            }
            StudioSlider {
                Layout.fillWidth: true
                studio: root.studio
                label: qsTr("B&W threshold")
                visible: root.monoOutput && imageFilters.ditherMode === 0
                from: 0
                to: 255
                liveUpdate: false
                value: displayOutput.monoThreshold
                onValueCommitted: (v) => displayOutput.setMonoThreshold(Math.round(v))
            }
        }

        StudioSection {
            studio: root.studio
            title: qsTr("Edges and mask")
            collapsible: true
            expanded: false

            StudioCheck {
                studio: root.studio
                text: qsTr("Sharpen")
                checked: imageFilters.sharpen
                onToggled: imageFilters.setSharpen(checked)
            }
            StudioSlider {
                Layout.fillWidth: true
                studio: root.studio
                label: qsTr("Sobel edges")
                from: 0; to: 100
                value: imageFilters.sobelEdges
                onValueCommitted: (v) => imageFilters.setSobelEdges(Math.round(v))
            }
            StudioSlider {
                Layout.fillWidth: true
                studio: root.studio
                label: qsTr("Posterize (gray)")
                from: 0; to: 30
                value: imageFilters.posterizeGray
                onValueCommitted: (v) => imageFilters.setPosterizeGray(Math.round(v))
            }
            StudioField { studio: root.studio; labelText: qsTr("Contours") }
            StudioCombo {
                id: contourCombo
                Layout.fillWidth: true
                studio: root.studio
                model: [
                    qsTr("None"),
                    qsTr("4-connected"),
                    qsTr("8-connected")
                ]
                Component.onCompleted: currentIndex = imageFilters.contourMode
                onActivated: imageFilters.setContourMode(currentIndex)
            }

            StudioCheck {
                id: maskOn
                studio: root.studio
                text: qsTr("Enable color mask")
                checked: imageFilters.colorMaskEnabled
                onToggled: imageFilters.setColorMaskEnabled(checked)
            }
            StudioTextField {
                Layout.fillWidth: true
                visible: maskOn.checked
                studio: root.studio
                text: imageFilters.maskColor
                placeholderText: "#RRGGBB"
                onEditingFinished: imageFilters.setMaskColor(text)
            }
            StudioSlider {
                Layout.fillWidth: true
                visible: maskOn.checked
                studio: root.studio
                label: qsTr("Tolerance")
                from: 0; to: 255
                value: imageFilters.maskTolerance
                onValueCommitted: (v) => imageFilters.setMaskTolerance(Math.round(v))
            }
            StudioSlider {
                Layout.fillWidth: true
                visible: maskOn.checked
                studio: root.studio
                label: qsTr("Amplify")
                from: 1; to: 10
                value: imageFilters.maskAmplify
                onValueCommitted: (v) => imageFilters.setMaskAmplify(Math.round(v))
            }
        }

        StudioButton {
            Layout.fillWidth: true
            studio: root.studio
            iconName: "eraser"
            text: qsTr("Reset all filters")
            onClicked: imageFilters.resetFilters()
        }

        Label {
            Layout.fillWidth: true
            visible: !converter.hasImage
            wrapMode: Text.WordWrap
            text: qsTr("Load an image to adjust placement and filters.")
            color: studio.textMuted
            font.pixelSize: studio.fontSizeSm
        }
    }

    Connections {
        target: imageTransform
        function onRotationChanged() { rotationSeg.selectedValue = imageTransform.rotation }
        function onScaleModeChanged() { scaleCombo.currentIndex = imageTransform.scaleMode }
    }

    Connections {
        target: displayOutput
        function onEncodingModeChanged() { encQuickCombo.syncEncQuick() }
    }

    Connections {
        target: imageFilters
        function onContourModeChanged() { contourCombo.currentIndex = imageFilters.contourMode }
        function onDitherModeChanged() {
            ditherCombo.currentIndex = imageFilters.ditherMode
            ditherComboTop.currentIndex = imageFilters.ditherMode
        }
        function onTonePresetChanged() { toneCombo.currentIndex = imageFilters.tonePreset }
    }
}
