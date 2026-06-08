import QtQuick
import QtQuick.Controls
import PixelStudio

pragma Translator: PixelStudio

StudioDialog {
    id: root

    property int bodyWidth: 440

    frameBorderWidth: 2
    frameRadius: 0
    frameDoubleBorder: true
    headerTitleColor: studio.text

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

    title: qsTr("About PixelStudio")
    modal: true
    anchors.centerIn: parent
    standardButtons: Dialog.NoButton
    width: bodyWidth + 2 * padding

    component SectionHeader: Row {
        id: headerRoot
        required property var studio
        property string label: ""

        width: parent.width
        spacing: headerRoot.studio.spacingSm

        Text {
            id: sectionLabel
            text: headerRoot.label
            font.family: headerRoot.studio.fontFamilyPixel
            font.pixelSize: headerRoot.studio.fontSizePixel
            color: headerRoot.studio.accent
        }

        PixelDottedRule {
            width: Math.max(0, headerRoot.width - sectionLabel.width - headerRoot.studio.spacingSm)
            height: 1
            lineColor: headerRoot.studio.accent
        }
    }

    component BodyText: Text {
        required property var studio
        property string body: ""
        width: parent.width
        wrapMode: Text.WordWrap
        text: body
        font.family: studio.fontFamily
        font.pixelSize: studio.fontSizeSm
        color: studio.text
        lineHeight: 1.45
    }

    component AboutLink: Item {
        id: link
        required property var studio
        property string label: ""
        property string url: ""
        property string iconName: ""

        width: parent.width
        height: Math.max(labelText.implicitHeight, 18)

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
            anchors.left: parent.left
            spacing: studio.spacingSm

            StudioIcon {
                anchors.verticalCenter: parent.verticalCenter
                name: link.iconName
                iconSize: 14
                tint: linkHover.hovered ? studio.accentHover : studio.accent
            }

            Text {
                id: labelText
                anchors.verticalCenter: parent.verticalCenter
                text: link.label
                font.family: studio.fontFamily
                font.pixelSize: studio.fontSizeSm
                color: linkHover.hovered ? studio.accentHover : studio.accent
                font.underline: true
            }
        }
    }

    component MetaRow: Row {
        required property var studio
        property string iconName: ""
        property string line: ""

        width: parent.width
        spacing: studio.spacingSm

        StudioIcon {
            visible: iconName.length > 0
            anchors.verticalCenter: parent.verticalCenter
            name: iconName
            iconSize: 14
            tint: studio.accent
        }

        Text {
            width: parent.width - (iconName.length > 0 ? 14 + studio.spacingSm : 0)
            wrapMode: Text.WordWrap
            text: line
            font.family: studio.fontFamily
            font.pixelSize: studio.fontSizeSm
            color: studio.text
        }
    }

    footer: Item {
        implicitHeight: studio.spacingLg + 46 + studio.spacingSm

        Button {
            id: okButton
            anchors.centerIn: parent
            implicitWidth: Math.max(152, implicitContentWidth + studio.spacing2xl * 2)
            implicitHeight: 46
            text: qsTr("OK")
            font.family: studio.fontFamilyPixel
            font.pixelSize: studio.fontSizePixel
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
                    border.color: Qt.rgba(0, 0, 0, 0.4)
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
        spacing: studio.spacingMd
        topPadding: studio.spacingSm
        bottomPadding: studio.spacingXs

        Item {
            width: parent.width
            height: heroRow.height

            Row {
                id: heroRow
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: studio.spacingLg

                Item {
                    width: 80
                    height: 72

                    AboutBrandMark {
                        id: brandMark
                        anchors.centerIn: parent
                        studio: root.studio
                    }

                    WelcomeSparkle {
                        anchors.right: brandMark.left
                        anchors.rightMargin: 2
                        anchors.verticalCenter: brandMark.verticalCenter
                        anchors.verticalCenterOffset: 4
                        tint: studio.accent
                        armLength: 4
                    }
                    WelcomeSparkle {
                        anchors.left: brandMark.right
                        anchors.leftMargin: 0
                        anchors.top: brandMark.top
                        anchors.topMargin: -2
                        tint: studio.accent
                        armLength: 5
                        sparkleOpacity: 0.9
                    }
                    WelcomeSparkle {
                        anchors.left: brandMark.right
                        anchors.leftMargin: 14
                        anchors.bottom: brandMark.bottom
                        anchors.bottomMargin: 10
                        tint: Qt.rgba(1, 1, 1, 0.45)
                        armLength: 4
                        sparkleOpacity: 0.5
                    }
                    WelcomeSparkle {
                        anchors.right: brandMark.left
                        anchors.rightMargin: 10
                        anchors.top: brandMark.top
                        anchors.topMargin: -4
                        tint: Qt.rgba(1, 1, 1, 0.35)
                        armLength: 4
                        sparkleOpacity: 0.4
                    }
                }

                Column {
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: studio.spacingSm

                    Row {
                        spacing: 0

                        Text {
                            text: qsTr("Pixel")
                            font.family: studio.fontFamilyPixel
                            font.pixelSize: studio.fontSizePixelLg
                            color: studio.text
                        }

                        Text {
                            text: qsTr("Studio")
                            font.family: studio.fontFamilyPixel
                            font.pixelSize: studio.fontSizePixelLg
                            color: studio.accent
                        }
                    }

                    Text {
                        text: qsTr("Version %1").arg(applicationVersion)
                        font.family: studio.fontFamilyPixel
                        font.pixelSize: studio.fontSizePixel
                        color: studio.accent
                    }
                }
            }
        }

        BodyText {
            studio: root.studio
            horizontalAlignment: Text.AlignHCenter
            body: qsTr("Convert your pixel art to amazing creations.")
        }

        SectionHeader {
            studio: root.studio
            label: qsTr("OVERVIEW")
        }

        BodyText {
            studio: root.studio
            body: qsTr("PixelStudio turns source images into firmware-ready assets for OLED, TFT and retro displays. Designed for pixel artists. Built for hardware makers.")
        }

        SectionHeader {
            studio: root.studio
            label: qsTr("LINKS")
        }

        Column {
            width: parent.width
            spacing: studio.spacingSm

            AboutLink {
                studio: root.studio
                iconName: "book"
                label: qsTr("Documentation")
                url: root.docsUrl
            }
            AboutLink {
                studio: root.studio
                iconName: "download"
                label: qsTr("Download releases")
                url: root.releasesUrl
            }
            AboutLink {
                studio: root.studio
                iconName: "github"
                label: qsTr("GitHub")
                url: root.repoUrl
            }
            AboutLink {
                studio: root.studio
                iconName: "bug"
                label: qsTr("Report issue")
                url: root.issuesUrl
            }
        }

        Row {
            width: parent.width
            spacing: studio.spacingMd

            readonly property real columnWidth: (width - 1 - spacing) / 2

            Column {
                id: creditsColumn
                width: parent.columnWidth
                spacing: studio.spacingSm

                SectionHeader {
                    width: parent.width
                    studio: root.studio
                    label: qsTr("CREDITS")
                }

                MetaRow {
                    studio: root.studio
                    iconName: "user"
                    line: qsTr("Producer: DeeTech")
                }

                MetaRow {
                    studio: root.studio
                    iconName: ""
                    line: qsTr("© %1 DeeTech. All rights reserved.").arg(new Date().getFullYear())
                }
            }

            Item {
                width: 1
                height: creditsColumn.height

                PixelDottedRule {
                    anchors.fill: parent
                    vertical: true
                    lineColor: root.studio.accent
                }
            }

            Column {
                width: parent.columnWidth
                spacing: studio.spacingSm

                SectionHeader {
                    width: parent.width
                    studio: root.studio
                    label: qsTr("SYSTEM")
                }

                MetaRow {
                    studio: root.studio
                    iconName: root.platformIcon
                    line: qsTr("Platform: %1").arg(root.platformLabel)
                }

                MetaRow {
                    studio: root.studio
                    iconName: "qt"
                    line: qsTr("Qt version: %1").arg(qtRuntimeVersion)
                }
            }
        }

        Item {
            width: parent.width
            height: licenseRow.height

            Row {
                id: licenseRow
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 0

                Text {
                    text: qsTr("PixelStudio is licensed under the ")
                    font.family: studio.fontFamily
                    font.pixelSize: studio.fontSizeSm
                    color: studio.text
                }

                Text {
                    text: qsTr("MIT License")
                    font.family: studio.fontFamily
                    font.pixelSize: studio.fontSizeSm
                    color: licenseHover.hovered ? studio.accentHover : studio.accent
                    font.underline: true

                    HoverHandler { id: licenseHover }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: Qt.openUrlExternally(root.licenseUrl)
                    }
                }

                Text {
                    text: "."
                    font.family: studio.fontFamily
                    font.pixelSize: studio.fontSizeSm
                    color: studio.text
                }
            }
        }
    }
}
