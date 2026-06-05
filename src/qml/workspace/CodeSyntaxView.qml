import QtQuick
import QtQuick.Controls

pragma Translator: PixelStudio

Item {
    id: root

    required property var studio
    property string sourceText: ""
    property bool wrap: false
    property string placeholderText: ""

    readonly property string highlightedHtml: {
        const theme = appSettings.codeSyntax
        const revision = theme.revision
        void revision
        return theme.highlight(sourceText)
    }

    TextArea {
        id: editor
        anchors.fill: parent
        readOnly: true
        textFormat: TextEdit.RichText
        text: root.sourceText.length > 0 ? root.highlightedHtml : ""
        font.family: studio.fontFamilyMono
        font.pixelSize: studio.fontSizeSm
        wrapMode: root.wrap ? TextArea.Wrap : TextArea.NoWrap
        selectByMouse: true
        background: null
    }

    Label {
        anchors.fill: parent
        anchors.margins: studio.spacingXs
        visible: root.sourceText.length === 0
        text: root.placeholderText
        font.family: studio.fontFamilyMono
        font.pixelSize: studio.fontSizeSm
        color: studio.textMuted
        wrapMode: Text.Wrap
        verticalAlignment: Text.AlignTop
    }
}
