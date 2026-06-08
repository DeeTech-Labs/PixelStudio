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

    readonly property real lineHeight: studio.fontSizeSm * 1.5
    readonly property int lineCount: {
        if (sourceText.length === 0)
            return 0
        let count = 1
        for (let i = 0; i < sourceText.length; ++i) {
            if (sourceText.charCodeAt(i) === 10)
                count++
        }
        return count
    }

    readonly property real documentHeight: Math.max(lineHeight, lineCount * lineHeight) + studio.spacingXs * 2
    readonly property real documentWidth: root.wrap
        ? Math.max(0, flickable.width - studio.spacingXs * 2)
        : Math.max(codeText.implicitWidth, Math.max(0, flickable.width - studio.spacingXs * 2))

    Flickable {
        id: flickable
        anchors.fill: parent
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        flickableDirection: root.wrap ? Flickable.VerticalFlick : Flickable.AutoFlickDirection
        contentWidth: root.wrap ? width : (documentWidth + studio.spacingXs * 2)
        contentHeight: root.sourceText.length > 0
            ? documentHeight
            : (placeholder.visible ? placeholder.implicitHeight + studio.spacingXs * 2 : 0)

        ScrollBar.vertical: ScrollBar {
            policy: ScrollBar.AsNeeded
        }

        ScrollBar.horizontal: ScrollBar {
            policy: root.wrap ? ScrollBar.AlwaysOff : ScrollBar.AsNeeded
        }

        Text {
            id: codeText
            x: studio.spacingXs
            y: studio.spacingXs
            width: root.documentWidth
            visible: root.sourceText.length > 0
            textFormat: Text.RichText
            text: root.highlightedHtml
            font.family: studio.fontFamilyMono
            font.pixelSize: studio.fontSizeSm
            wrapMode: root.wrap ? Text.Wrap : Text.NoWrap
            layer.enabled: true
            layer.smooth: false
        }

        Label {
            id: placeholder
            x: studio.spacingXs
            y: studio.spacingXs
            width: Math.max(0, flickable.width - studio.spacingXs * 2)
            visible: root.sourceText.length === 0
            text: root.placeholderText
            font.family: studio.fontFamilyMono
            font.pixelSize: studio.fontSizeSm
            color: studio.textMuted
            wrapMode: Text.Wrap
            verticalAlignment: Text.AlignTop
        }
    }
}
