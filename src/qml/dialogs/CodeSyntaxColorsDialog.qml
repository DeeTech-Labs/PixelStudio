import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import PixelStudio

pragma Translator: PixelStudio

RetroDialog {
    id: root

    title: qsTr("Code syntax colors")
    modal: true
    standardButtons: Dialog.Ok
    width: 500
    property string activeKey: ""

    ColorDialog {
        id: colorPicker
        title: qsTr("Pick color")
        onAccepted: applyPickedColor(selectedColor)
    }

    function applyPickedColor(color) {
        if (!color || !root.activeKey)
            return
        const hex = color.toString()
        const theme = appSettings.codeSyntax
        switch (root.activeKey) {
        case "comment": theme.comment = hex; break
        case "directive": theme.directive = hex; break
        case "keyword": theme.keyword = hex; break
        case "type": theme.type = hex; break
        case "number": theme.number = hex; break
        case "string": theme.string = hex; break
        case "macro": theme.macro = hex; break
        case "identifier": theme.identifier = hex; break
        case "punctuation": theme.punctuation = hex; break
        case "text": theme.text = hex; break
        }
    }

    function openPicker(key, currentColor) {
        root.activeKey = key
        colorPicker.selectedColor = currentColor
        colorPicker.open()
    }

    component ColorRow: RowLayout {
        id: row
        required property var studio
        required property string label
        required property string colorKey
        required property string colorValue
        Layout.fillWidth: true
        spacing: row.studio.spacingSm

        Label {
            Layout.preferredWidth: 148
            text: row.label
            font.pixelSize: row.studio.fontSizeSm
            color: row.studio.textSecondary
            elide: Text.ElideRight
        }

        Rectangle {
            Layout.preferredWidth: 28
            Layout.preferredHeight: 28
            color: row.colorValue
            border.width: 1
            border.color: row.studio.border

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: root.openPicker(row.colorKey, row.colorValue)
            }
        }

        StudioTextField {
            Layout.fillWidth: true
            studio: row.studio
            text: row.colorValue
            font.family: row.studio.fontFamilyMono
            onEditingFinished: applyPickedColor(Qt.color(text))
        }
    }

    ColumnLayout {
        width: parent.width
        spacing: studio.spacingMd

        Label {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: qsTr("VS Code–style highlighting for generated C/C++ code. Changes apply immediately.")
            font.pixelSize: studio.fontSizeSm
            color: studio.textMuted
        }

        ScrollView {
            Layout.fillWidth: true
            Layout.preferredHeight: 360
            clip: true

            ColumnLayout {
                width: parent.width
                spacing: studio.spacingXs

                ColorRow {
                    studio: root.studio
                    label: qsTr("Comments")
                    colorKey: "comment"
                    colorValue: appSettings.codeSyntax.comment
                }
                ColorRow {
                    studio: root.studio
                    label: qsTr("Preprocessor")
                    colorKey: "directive"
                    colorValue: appSettings.codeSyntax.directive
                }
                ColorRow {
                    studio: root.studio
                    label: qsTr("Keywords")
                    colorKey: "keyword"
                    colorValue: appSettings.codeSyntax.keyword
                }
                ColorRow {
                    studio: root.studio
                    label: qsTr("Types")
                    colorKey: "type"
                    colorValue: appSettings.codeSyntax.type
                }
                ColorRow {
                    studio: root.studio
                    label: qsTr("Numbers")
                    colorKey: "number"
                    colorValue: appSettings.codeSyntax.number
                }
                ColorRow {
                    studio: root.studio
                    label: qsTr("Strings")
                    colorKey: "string"
                    colorValue: appSettings.codeSyntax.string
                }
                ColorRow {
                    studio: root.studio
                    label: qsTr("Macros")
                    colorKey: "macro"
                    colorValue: appSettings.codeSyntax.macro
                }
                ColorRow {
                    studio: root.studio
                    label: qsTr("Identifiers")
                    colorKey: "identifier"
                    colorValue: appSettings.codeSyntax.identifier
                }
                ColorRow {
                    studio: root.studio
                    label: qsTr("Punctuation")
                    colorKey: "punctuation"
                    colorValue: appSettings.codeSyntax.punctuation
                }
                ColorRow {
                    studio: root.studio
                    label: qsTr("Plain text")
                    colorKey: "text"
                    colorValue: appSettings.codeSyntax.text
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: preview.height + studio.spacingSm * 2
            color: studio.surfaceInset
            border.width: 1
            border.color: studio.border
            radius: studio.radiusSm

            CodeSyntaxView {
                id: preview
                anchors.fill: parent
                anchors.margins: studio.spacingSm
                studio: root.studio
                wrap: true
                sourceText: "// Preview\n#define IMAGE_DATA_WIDTH 128\nstatic const uint8_t image_data[] PROGMEM = {\n    0x00, 0xff, 0xfe\n};"
            }
        }

        StudioButton {
            studio: root.studio
            text: qsTr("Reset to VS Code defaults")
            onClicked: appSettings.codeSyntax.resetDefaults()
        }
    }
}
