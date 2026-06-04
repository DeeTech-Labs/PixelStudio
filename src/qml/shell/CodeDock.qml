import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import PixelStudio

pragma Translator: PixelStudio

Rectangle {
    id: root
    required property var studio
    property var onSaveCode: null
    property var onSaveBin: null

    color: studio.surface
    border.width: 1
    border.color: studio.divider

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: studio.spacingSm
        spacing: studio.spacingSm

        RowLayout {
            Layout.fillWidth: true
            Label {
                text: qsTr("Generated code")
                font.weight: Font.DemiBold
                font.pixelSize: studio.fontSizeSm
                color: studio.text
            }
            Label {
                text: converter.encodingModeName
                font.pixelSize: studio.fontSizeXs
                color: studio.textMuted
            }
            Rectangle {
                visible: converter.generatedCode.length > 0
                implicitWidth: badge.implicitWidth + studio.spacingSm * 2
                implicitHeight: 18
                radius: studio.radiusPill
                color: studio.accentSoft
                Label {
                    id: badge
                    anchors.centerIn: parent
                    text: converter.dataByteCount + " B"
                    font.pixelSize: studio.fontSizeXs
                    color: studio.accent
                }
            }
            Item { Layout.fillWidth: true }
            StudioButton {
                visible: converter.generatedCodeTruncated
                studio: root.studio
                compact: true
                text: converter.showFullGeneratedCode ? qsTr("Preview") : qsTr("Show all")
                onClicked: converter.setShowFullGeneratedCode(!converter.showFullGeneratedCode)
            }
            StudioButton {
                studio: root.studio
                compact: true
                iconName: "download"
                text: qsTr(".h")
                enabled: converter.generatedCode.length > 0
                onClicked: if (onSaveCode) onSaveCode()
            }
            StudioButton {
                studio: root.studio
                compact: true
                iconName: "file-image"
                text: qsTr(".bin")
                enabled: converter.hasPreview
                onClicked: if (onSaveBin) onSaveBin()
            }
            StudioButton {
                studio: root.studio
                primary: true
                compact: true
                iconName: "copy"
                text: qsTr("Copy")
                enabled: converter.generatedCode.length > 0
                onClicked: converter.copyToClipboard(converter.generatedCode)
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: studio.radiusMd
            color: studio.surfaceInset
            border.width: 1
            border.color: studio.divider
            clip: true

            ScrollView {
                anchors.fill: parent
                anchors.margins: studio.spacingXs
                clip: true
                TextArea {
                    readOnly: true
                    text: converter.generatedCodePreview
                    font.family: studio.fontFamilyMono
                    font.pixelSize: studio.fontSizeSm
                    color: studio.text
                    wrapMode: appSettings.codeWrap ? TextArea.Wrap : TextArea.NoWrap
                    selectByMouse: true
                    placeholderText: qsTr("Code appears here after you import an image.")
                    placeholderTextColor: studio.textMuted
                    background: null
                }
            }
        }
    }
}
