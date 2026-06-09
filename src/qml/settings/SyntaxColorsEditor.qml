import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import PixelStudio

pragma Translator: Shell


Item {
    id: root

    required property var studio

    property string activeKey: ""
    property int previewHeight: 96

    implicitHeight: column.implicitHeight
    Layout.fillWidth: true

    ColorDialog {
        id: colorPicker
        title: qsTr("Pick color")
        onAccepted: applyPickedColor(selectedColor)
    }

    function applyPickedColor(color) {
        if (!color || !root.activeKey)
            return
        const hex = color.toString()
        const theme = workspace.settings.codeSyntax
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
            Layout.minimumWidth: 112
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
        id: column
        width: parent.width
        spacing: studio.spacingMd

        Label {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: qsTr("VS Code–style highlighting for generated C/C++ code. Changes apply immediately.")
            font.pixelSize: studio.fontSizeSm
            color: studio.textMuted
        }

        GridLayout {
            id: colorGrid
            Layout.fillWidth: true
            columns: colorGrid.width >= 640 ? 2 : 1
            columnSpacing: studio.spacingLg
            rowSpacing: studio.spacingXs
            width: parent.width

            ColorRow {
                studio: root.studio
                label: qsTr("Comments")
                colorKey: "comment"
                colorValue: workspace.settings.codeSyntax.comment
            }
            ColorRow {
                studio: root.studio
                label: qsTr("Preprocessor")
                colorKey: "directive"
                colorValue: workspace.settings.codeSyntax.directive
            }
            ColorRow {
                studio: root.studio
                label: qsTr("Keywords")
                colorKey: "keyword"
                colorValue: workspace.settings.codeSyntax.keyword
            }
            ColorRow {
                studio: root.studio
                label: qsTr("Types")
                colorKey: "type"
                colorValue: workspace.settings.codeSyntax.type
            }
            ColorRow {
                studio: root.studio
                label: qsTr("Numbers")
                colorKey: "number"
                colorValue: workspace.settings.codeSyntax.number
            }
            ColorRow {
                studio: root.studio
                label: qsTr("Strings")
                colorKey: "string"
                colorValue: workspace.settings.codeSyntax.string
            }
            ColorRow {
                studio: root.studio
                label: qsTr("Macros")
                colorKey: "macro"
                colorValue: workspace.settings.codeSyntax.macro
            }
            ColorRow {
                studio: root.studio
                label: qsTr("Identifiers")
                colorKey: "identifier"
                colorValue: workspace.settings.codeSyntax.identifier
            }
            ColorRow {
                studio: root.studio
                label: qsTr("Punctuation")
                colorKey: "punctuation"
                colorValue: workspace.settings.codeSyntax.punctuation
            }
            ColorRow {
                studio: root.studio
                label: qsTr("Plain text")
                colorKey: "text"
                colorValue: workspace.settings.codeSyntax.text
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: root.previewHeight
            color: studio.surfaceInset
            border.width: 1
            border.color: studio.border
            radius: studio.radiusSm
            clip: true

            CodeSyntaxView {
                anchors.fill: parent
                anchors.margins: studio.spacingSm
                studio: root.studio
                wrap: true
                sourceText: "// Preview\n#define IMAGE_DATA_WIDTH 128\nstatic const uint8_t image_data[] PROGMEM = {\n    0x00, 0xff, 0xfe\n};"
            }
        }

        StudioButton {
            Layout.fillWidth: true
            studio: root.studio
            text: qsTr("Reset to VS Code defaults")
            onClicked: workspace.settings.codeSyntax.resetDefaults()
        }
    }
}
