import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import PixelStudio

pragma Translator: Shell


StudioDialog {
    id: root

    required property var win

    property int bodyWidth: win
        ? win.dialogBodyWidth(420, studio.spacingXl * 4)
        : 420

    showCornerBrackets: true
    frameDoubleBorder: true
    frameRadius: 0
    frameBorderWidth: 2
    headerTitleElide: Text.ElideNone
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    width: bodyWidth + 2 * padding
    title: qsTr("Exit PixelStudio?")
    modal: true
    standardButtons: Dialog.NoButton

    FontLoader {
        id: pixelFontLoader
        source: "qrc:/fonts/PressStart2P-Regular.ttf"
    }

    readonly property string pixelFont: pixelFontLoader.status === FontLoader.Ready
        ? pixelFontLoader.name
        : studio.fontFamilyPixel

    readonly property int actionButtonHeight: 44

    component PixelActionButton: Button {
        id: actionButton
        required property var studioRef
        required property string pixelFontName
        property bool primaryAction: false

        flat: true
        implicitHeight: root.actionButtonHeight
        implicitWidth: Math.max(112, implicitContentWidth + studioRef.spacing2xl * 2)
        height: root.actionButtonHeight
        topPadding: 0
        bottomPadding: 0
        leftPadding: studioRef.spacingLg
        rightPadding: studioRef.spacingLg
        font.family: pixelFontName
        font.pixelSize: studioRef.fontSizePixel

        HoverHandler { id: actionHover }

        background: Item {
            Rectangle {
                anchors.fill: parent
                color: {
                    if (!actionButton.enabled)
                        return studioRef.surfaceInset
                    if (actionButton.primaryAction) {
                        if (actionButton.pressed)
                            return studioRef.accentPressed
                        if (actionHover.hovered)
                            return studioRef.accentHover
                        return studioRef.accent
                    }
                    if (actionButton.pressed)
                        return studioRef.surfaceHover
                    if (actionHover.hovered)
                        return studioRef.surfaceRaised
                    return studioRef.surfaceInset
                }
                border.width: 2
                border.color: actionButton.primaryAction
                    ? studioRef.accentOnAccent
                    : studioRef.accent
            }

            Rectangle {
                anchors.fill: parent
                anchors.margins: 4
                color: "transparent"
                border.width: 1
                border.color: Qt.rgba(0, 0, 0, actionButton.primaryAction ? 0.45 : 0.3)
            }

            Repeater {
                model: actionButton.primaryAction ? 4 : 0
                delegate: Rectangle {
                    x: (index % 2 === 0) ? 2 : (parent.width - 5)
                    y: (index < 2) ? 2 : (parent.height - 5)
                    width: 3
                    height: 3
                    color: studioRef.accentOnAccent
                }
            }
        }

        contentItem: Text {
            anchors.fill: parent
            text: actionButton.text
            font: actionButton.font
            color: {
                if (!actionButton.enabled)
                    return studioRef.textMuted
                if (actionButton.primaryAction)
                    return studioRef.accentOnAccent
                return studioRef.text
            }
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }

    contentItem: ColumnLayout {
        width: root.bodyWidth
        spacing: studio.spacingLg

        Rectangle {
            Layout.fillWidth: true
            implicitHeight: warningRow.implicitHeight + studio.spacingLg * 2
            radius: studio.radiusSm
            color: studio.surfaceInset
            border.width: 1
            border.color: studio.border

            Rectangle {
                width: 4
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                color: studio.warning
            }

            RowLayout {
                id: warningRow
                anchors.fill: parent
                anchors.margins: studio.spacingLg
                anchors.leftMargin: studio.spacingLg + 6
                spacing: studio.spacingLg

                Item {
                    Layout.preferredWidth: 48
                    Layout.preferredHeight: 48

                    Rectangle {
                        anchors.fill: parent
                        color: Qt.rgba(studio.warning.r, studio.warning.g, studio.warning.b, 0.14)
                        border.width: 2
                        border.color: studio.warning
                    }

                    Text {
                        anchors.centerIn: parent
                        text: "!"
                        font.family: root.pixelFont
                        font.pixelSize: root.studio.fontSizePixelLg
                        color: studio.warning
                    }
                }

                Text {
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    text: qsTr("Close the application? Unsaved exported files or project changes may be lost.")
                    font.family: studio.fontFamily
                    font.pixelSize: studio.fontSizeBase
                    lineHeight: 1.45
                    color: studio.text
                }
            }
        }

        PixelDottedRule {
            Layout.fillWidth: true
            height: 1
            lineColor: studio.accent
        }
    }

    footer: Item {
        implicitHeight: root.actionButtonHeight + studio.spacingXl

        RowLayout {
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.rightMargin: studio.spacingXl
            anchors.bottomMargin: studio.spacingLg
            spacing: studio.spacingMd

            PixelActionButton {
                Layout.preferredHeight: root.actionButtonHeight
                Layout.alignment: Qt.AlignVCenter
                studioRef: studio
                pixelFontName: root.pixelFont
                text: qsTr("No")
                onClicked: root.reject()
            }

            PixelActionButton {
                Layout.preferredHeight: root.actionButtonHeight
                Layout.alignment: Qt.AlignVCenter
                studioRef: studio
                pixelFontName: root.pixelFont
                primaryAction: true
                text: qsTr("Yes")
                onClicked: root.accept()
            }
        }
    }
}
