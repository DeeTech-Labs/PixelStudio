import QtQuick
import QtQuick.Controls
import PixelStudio

pragma Translator: PixelStudio

StudioDialog {
    id: root

    property int bodyWidth: 420

    readonly property string repoUrl: "https://github.com/DeeTech-Labs/PixelStudio"
    readonly property string docsUrl: repoUrl + "/blob/main/docs/README.md"
    readonly property string issuesUrl: repoUrl + "/issues"
    readonly property string releasesUrl: repoUrl + "/releases"
    readonly property string licenseUrl: repoUrl + "/blob/main/LICENSE"
    readonly property string noticeUrl: repoUrl + "/blob/main/NOTICE.txt"

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

    title: qsTr("About PixelStudio")
    modal: true
    anchors.centerIn: parent
    standardButtons: Dialog.NoButton
    width: bodyWidth + 2 * padding

    component SectionLabel: Text {
        required property var studio
        property string label: ""
        width: parent.width
        text: label
        font.family: studio.fontFamilyPixel
        font.pixelSize: studio.fontSizePixel
        color: studio.accent
        opacity: 0.9
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

    component MetaLine: Text {
        required property var studio
        property string line: ""
        width: parent.width
        horizontalAlignment: Text.AlignHCenter
        text: line
        font.family: studio.fontFamily
        font.pixelSize: studio.fontSizeXs
        color: studio.textSecondary
    }

    component AboutLink: Item {
        id: link
        required property var studio
        property string label: ""
        property string url: ""
        property string iconName: ""

        width: parent.width
        height: Math.max(labelText.implicitHeight, 18)

        signal activated()

        HoverHandler { id: linkHover }

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                if (link.url.length > 0)
                    Qt.openUrlExternally(link.url)
                link.activated()
            }
        }

        Row {
            anchors.left: parent.left
            spacing: studio.spacingSm

            StudioIcon {
                visible: link.iconName.length > 0
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
                font.underline: linkHover.hovered
            }
        }
    }

    footer: Item {
        implicitHeight: studio.spacingLg + 40 + studio.spacingSm

        Button {
            id: okButton
            anchors.centerIn: parent
            implicitWidth: Math.max(120, implicitContentWidth + studio.spacing2xl * 2)
            implicitHeight: 40
            text: qsTr("OK")
            font.family: studio.fontFamilyPixel
            font.pixelSize: studio.fontSizePixel
            onClicked: root.accept()

            HoverHandler { id: okHover }

            background: Rectangle {
                radius: studio.radiusMd
                color: {
                    if (!okButton.enabled)
                        return studio.surfaceInset
                    if (okButton.pressed)
                        return studio.accentPressed
                    if (okHover.hovered)
                        return studio.accentHover
                    return studio.accent
                }
                gradient: okButton.enabled && !okButton.pressed ? okGrad : null
                Gradient {
                    id: okGrad
                    orientation: Gradient.Vertical
                    GradientStop { position: 0.0; color: studio.accentTop }
                    GradientStop { position: 1.0; color: studio.accentBottom }
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
            height: 88

            WelcomeBrandMark {
                id: brandMark
                anchors.centerIn: parent
            }

            WelcomeSparkle {
                anchors.right: brandMark.left
                anchors.rightMargin: 10
                anchors.verticalCenter: brandMark.verticalCenter
                anchors.verticalCenterOffset: 8
                tint: studio.accent
                armLength: 4
            }
            WelcomeSparkle {
                anchors.left: brandMark.right
                anchors.leftMargin: 6
                anchors.top: brandMark.top
                anchors.topMargin: 4
                tint: studio.accent
                armLength: 5
                sparkleOpacity: 0.8
            }
            WelcomeSparkle {
                anchors.left: brandMark.right
                anchors.leftMargin: 20
                anchors.bottom: brandMark.bottom
                anchors.bottomMargin: 18
                tint: Qt.rgba(1, 1, 1, 0.4)
                armLength: 4
                sparkleOpacity: 0.45
            }
            WelcomeSparkle {
                anchors.left: brandMark.left
                anchors.leftMargin: 24
                anchors.bottom: brandMark.top
                anchors.bottomMargin: 10
                tint: Qt.rgba(1, 1, 1, 0.35)
                armLength: 4
                sparkleOpacity: 0.35
            }
        }

        Item {
            width: parent.width
            height: titleRow.height

            Row {
                id: titleRow
                anchors.horizontalCenter: parent.horizontalCenter
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
        }

        Text {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            text: qsTr("Version %1").arg(applicationVersion)
            font.family: studio.fontFamilyPixel
            font.pixelSize: studio.fontSizePixel
            color: studio.accent
        }

        BodyText {
            studio: root.studio
            horizontalAlignment: Text.AlignHCenter
            body: qsTr("Convert pixel art to OLED, TFT and retro displays with ease.")
        }

        PixelDivider {
            width: parent.width
            studio: root.studio
        }

        SectionLabel {
            studio: root.studio
            label: qsTr("Overview")
        }

        BodyText {
            studio: root.studio
            body: qsTr("Desktop studio for turning images into C/C++ firmware arrays and binary exports. Display profiles, batch export, sprite atlas, and .pspx project files.")
        }

        PixelDivider {
            width: parent.width
            studio: root.studio
        }

        SectionLabel {
            studio: root.studio
            label: qsTr("Links")
        }

        Column {
            width: parent.width
            spacing: studio.spacingXs

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
                iconName: "folder-open"
                label: qsTr("Source code on GitHub")
                url: root.repoUrl
            }
            AboutLink {
                studio: root.studio
                iconName: "sparkles"
                label: qsTr("Report an issue")
                url: root.issuesUrl
            }
        }

        PixelDivider {
            width: parent.width
            studio: root.studio
        }

        SectionLabel {
            studio: root.studio
            label: qsTr("Credits")
        }

        MetaLine {
            studio: root.studio
            line: qsTr("Producer: DeeTech")
        }
        MetaLine {
            studio: root.studio
            line: qsTr("© %1 DeeTech Labs").arg(new Date().getFullYear())
        }

        PixelDivider {
            width: parent.width
            studio: root.studio
        }

        SectionLabel {
            studio: root.studio
            label: qsTr("System")
        }

        Column {
            width: parent.width
            spacing: studio.spacingXs

            MetaLine {
                studio: root.studio
                line: qsTr("Platform: %1").arg(root.platformLabel)
            }
            MetaLine {
                studio: root.studio
                line: qsTr("Qt %1").arg(qtRuntimeVersion)
            }
        }

        BodyText {
            studio: root.studio
            horizontalAlignment: Text.AlignHCenter
            color: studio.textMuted
            font.pixelSize: studio.fontSizeXs
            body: qsTr("Distributed under the MIT License.")
        }

        Column {
            width: parent.width
            spacing: studio.spacingXs

            AboutLink {
                studio: root.studio
                label: qsTr("View license")
                url: root.licenseUrl
            }
            AboutLink {
                studio: root.studio
                label: qsTr("Third-party notices")
                url: root.noticeUrl
            }
        }
    }
}
