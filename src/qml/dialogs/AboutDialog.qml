import QtQuick
import QtQuick.Controls
import PixelStudio

pragma Translator: Shell


StudioDialog {
    id: root

    property int bodyWidth: 440

    showCornerBrackets: false
    frameBorderWidth: 2
    frameRadius: 0
    frameDoubleBorder: true
    headerTitleColor: studio.text
    headerTitleElide: Text.ElideNone

    readonly property string repoUrl: "https://github.com/DeeTech-Labs/PixelStudio"
    readonly property string docsUrl: repoUrl + "/blob/main/docs/README.md"
    readonly property string issuesUrl: repoUrl + "/issues"
    readonly property string releasesUrl: repoUrl + "/releases"
    readonly property string licenseUrl: repoUrl + "/blob/main/LICENSE"

    readonly property string platformLabel: {
        switch (Qt.platform.os) {
        case "windows":
            return qsTr("Windows")
        case "osx":
            return qsTr("macOS")
        case "linux":
            return qsTr("Linux")
        default:
            return Qt.platform.os
        }
    }

    readonly property string platformIcon: {
        switch (Qt.platform.os) {
        case "windows":
            return "windows"
        case "osx":
            return "display"
        case "linux":
            return "chip"
        default:
            return "display"
        }
    }

    FontLoader {
        id: aboutPixelFont
        source: "qrc:/fonts/PressStart2P-Regular.ttf"
    }

    readonly property string pixelFont: aboutPixelFont.status === FontLoader.Ready
        ? aboutPixelFont.name
        : studio.fontFamilyPixel

    title: qsTr("About PixelStudio")
    modal: true
    standardButtons: Dialog.NoButton
    width: bodyWidth + 2 * padding

    component SectionHeader: Row {
        id: headerRoot
        required property var studio
        required property string pixelFont
        property string label: ""

        width: parent.width
        spacing: headerRoot.studio.spacingSm
        height: sectionLabel.height

        Text {
            id: sectionLabel
            text: headerRoot.label
            font.family: headerRoot.pixelFont
            font.pixelSize: headerRoot.studio.fontSizePixel
            color: headerRoot.studio.accent
        }

        PixelDottedRule {
            anchors.verticalCenter: parent.verticalCenter
            width: Math.max(0, headerRoot.width - sectionLabel.width - headerRoot.studio.spacingSm)
            height: 1
            lineColor: headerRoot.studio.accent
        }
    }

    component BodyText: Text {
        required property var studio
        required property string pixelFont
        property string body: ""
        width: parent.width
        wrapMode: Text.WordWrap
        text: body
        font.family: pixelFont
        font.pixelSize: 8
        color: studio.text
        lineHeight: 1.75
    }

    component AboutLink: Item {
        id: link
        required property var studio
        required property string pixelFont
        property string label: ""
        property string url: ""
        property string iconName: ""

        width: parent.width
        height: linkRow.height

        HoverHandler { id: linkHover }

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                if (link.url.length > 0)
                    Qt.openUrlExternally(link.url)
            }
        }

        Row {
            id: linkRow
            spacing: studio.spacingSm

            Item {
                width: 14
                height: labelText.implicitHeight

                StudioIcon {
                    anchors.centerIn: parent
                    name: link.iconName
                    iconSize: 14
                    tint: linkHover.hovered ? studio.accentHover : studio.accent
                }
            }

            Text {
                id: labelText
                text: link.label
                font.family: pixelFont
                font.pixelSize: 8
                color: linkHover.hovered ? studio.accentHover : studio.accent
                font.underline: true
            }
        }
    }

    component MetaRow: Item {
        id: metaRow
        required property var studio
        required property string pixelFont
        property string iconName: ""
        property string line: ""

        width: parent.width
        readonly property real iconSlot: iconName.length > 0 ? 14 + studio.spacingSm : 0
        implicitHeight: metaText.implicitHeight
        height: implicitHeight

        StudioIcon {
            visible: metaRow.iconName.length > 0
            x: 0
            y: Math.max(0, (metaRow.height - 14) / 2)
            name: metaRow.iconName
            iconSize: 14
            tint: studio.accent
        }

        Text {
            id: metaText
            x: metaRow.iconSlot
            width: metaRow.width - metaRow.iconSlot
            wrapMode: Text.WordWrap
            text: line
            font.family: pixelFont
            font.pixelSize: 8
            lineHeight: 1.6
            color: studio.text
        }
    }

    footer: Item {
        implicitHeight: studio.spacingLg + 48 + studio.spacingSm

        Button {
            id: okButton
            anchors.centerIn: parent
            implicitWidth: Math.max(168, implicitContentWidth + studio.spacing2xl * 2)
            implicitHeight: 48
            text: qsTr("OK")
            font.family: root.pixelFont
            font.pixelSize: root.studio.fontSizePixel
            onClicked: root.accept()

            HoverHandler { id: okHover }

            background: Item {
                Rectangle {
                    anchors.fill: parent
                    color: {
                        if (!okButton.enabled)
                            return studio.surfaceInset
                        if (okButton.pressed)
                            return studio.accentPressed
                        if (okHover.hovered)
                            return studio.accentHover
                        return studio.accent
                    }
                    border.width: 3
                    border.color: studio.accentOnAccent
                }

                Rectangle {
                    anchors.fill: parent
                    anchors.margins: 5
                    color: "transparent"
                    border.width: 1
                    border.color: Qt.rgba(0, 0, 0, 0.45)
                }

                Repeater {
                    model: 4
                    delegate: Rectangle {
                        x: (index % 2 === 0) ? 2 : (parent.width - 5)
                        y: (index < 2) ? 2 : (parent.height - 5)
                        width: 3
                        height: 3
                        color: studio.accentOnAccent
                    }
                }
            }

            contentItem: Text {
                text: okButton.text
                font: okButton.font
                color: okButton.enabled ? studio.accentOnAccent : studio.textMuted
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }
    }

    contentItem: Column {
        width: root.bodyWidth
        spacing: studio.spacingLg
        topPadding: studio.spacingSm
        bottomPadding: studio.spacingSm
        clip: true

        Column {
            width: parent.width
            spacing: studio.spacingMd

            Row {
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: studio.spacingLg

                AboutBrandMark {
                    anchors.verticalCenter: parent.verticalCenter
                    markSize: 64
                }

                Column {
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: studio.spacingSm

                    Text {
                        text: qsTr("PixelStudio")
                        font.family: root.pixelFont
                        font.pixelSize: studio.fontSizePixelLg
                        color: studio.accent
                    }

                    Text {
                        text: qsTr("Version %1").arg(applicationVersion)
                        font.family: root.pixelFont
                        font.pixelSize: studio.fontSizePixel
                        color: studio.accent
                    }
                }
            }

            Text {
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                text: qsTr("Convert pixel art to OLED, TFT and retro displays with ease.")
                font.family: root.pixelFont
                font.pixelSize: 8
                color: studio.text
                lineHeight: 1.6
            }
        }

        SectionHeader {
            studio: root.studio
            pixelFont: root.pixelFont
            label: qsTr("Overview")
        }

        BodyText {
            studio: root.studio
            pixelFont: root.pixelFont
            body: qsTr("Desktop studio for turning images into C/C++ firmware arrays and binary exports. Display profiles, batch export, sprite atlas, and .pspx project files.")
        }

        SectionHeader {
            studio: root.studio
            pixelFont: root.pixelFont
            label: qsTr("Links")
        }

        Column {
            width: parent.width
            spacing: studio.spacingMd

            AboutLink {
                studio: root.studio
                pixelFont: root.pixelFont
                iconName: "book"
                label: qsTr("Documentation")
                url: root.docsUrl
            }
            AboutLink {
                studio: root.studio
                pixelFont: root.pixelFont
                iconName: "download"
                label: qsTr("Download releases")
                url: root.releasesUrl
            }
            AboutLink {
                studio: root.studio
                pixelFont: root.pixelFont
                iconName: "github"
                label: qsTr("Source code on GitHub")
                url: root.repoUrl
            }
            AboutLink {
                studio: root.studio
                pixelFont: root.pixelFont
                iconName: "bug"
                label: qsTr("Report an issue")
                url: root.issuesUrl
            }
        }

        Row {
            width: parent.width
            spacing: studio.spacingMd

            readonly property real columnWidth: (width - 1 - spacing) / 2
            readonly property real splitHeight: Math.max(creditsColumn.height, systemColumn.height)

            Column {
                id: creditsColumn
                width: parent.columnWidth
                spacing: studio.spacingMd

                SectionHeader {
                    width: parent.width
                    studio: root.studio
                    pixelFont: root.pixelFont
                    label: qsTr("Credits")
                }

                MetaRow {
                    studio: root.studio
                    pixelFont: root.pixelFont
                    iconName: "user"
                    line: qsTr("Producer: DeeTech")
                }

                MetaRow {
                    studio: root.studio
                    pixelFont: root.pixelFont
                    iconName: "calendar"
                    line: qsTr("© %1 DeeTech Labs").arg(new Date().getFullYear())
                }
            }

            Item {
                width: 1
                height: parent.splitHeight

                PixelDottedRule {
                    anchors.fill: parent
                    vertical: true
                    lineColor: root.studio.accent
                }
            }

            Column {
                id: systemColumn
                width: parent.columnWidth
                spacing: studio.spacingMd

                SectionHeader {
                    width: parent.width
                    studio: root.studio
                    pixelFont: root.pixelFont
                    label: qsTr("System")
                }

                MetaRow {
                    studio: root.studio
                    pixelFont: root.pixelFont
                    iconName: root.platformIcon
                    line: qsTr("Platform: %1").arg(root.platformLabel)
                }

                MetaRow {
                    studio: root.studio
                    pixelFont: root.pixelFont
                    iconName: "qt"
                    line: qsTr("Qt %1").arg(qtRuntimeVersion)
                }
            }
        }

        Item {
            width: parent.width
            height: licenseText.height

            HoverHandler { id: licenseHover }

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: Qt.openUrlExternally(root.licenseUrl)
            }

            Text {
                id: licenseText
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                text: qsTr("Distributed under the MIT License.")
                font.family: root.pixelFont
                font.pixelSize: 8
                lineHeight: 1.6
                color: licenseHover.hovered ? studio.accentHover : studio.text
            }
        }
    }
}
