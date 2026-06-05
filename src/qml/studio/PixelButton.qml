import QtQuick
import QtQuick.Controls

pragma Translator: PixelStudio

Button {
    id: root

    required property var studio
    property bool primary: false
    property bool compact: false
    property string iconName: ""

    implicitHeight: compact ? studio.controlHeightSm : (primary ? 60 : studio.controlHeight)
    leftPadding: compact ? studio.spacingMd : studio.spacingLg
    rightPadding: compact ? studio.spacingMd : studio.spacingLg

    font.family: primary ? studio.fontFamilyPixel : studio.fontFamily
    font.pixelSize: primary ? studio.fontSizePixel : (compact ? studio.fontSizeSm : studio.fontSizeBase)

    background: Item {
        Rectangle {
            visible: primary
            anchors.fill: parent
            anchors.topMargin: 6
            radius: studio.radiusMd
            color: studio.accentDark
        }
        Rectangle {
            anchors.fill: parent
            radius: studio.radiusMd
            color: {
                if (!root.enabled) return studio.surfaceInset
                if (primary) {
                    if (root.pressed) return studio.accentPressed
                    return studio.accent
                }
                if (root.pressed) return studio.surfaceHover
                if (hover.hovered) return studio.surfaceRaised
                return studio.surfaceInput
            }
            border.width: primary ? 0 : 1
            border.color: primary ? studio.accent : studio.border
            gradient: primary && root.enabled ? grad : null
            Gradient {
                id: grad
                orientation: Gradient.Vertical
                GradientStop { position: 0.0; color: studio.accentTop }
                GradientStop { position: 1.0; color: studio.accentBottom }
            }
        }
    }

    contentItem: Row {
        anchors.centerIn: parent
        spacing: iconName.length > 0 ? studio.spacingSm : 0

        StudioIcon {
            visible: root.iconName.length > 0
            name: root.iconName
            iconSize: root.compact ? 14 : 16
            tint: root.enabled
                ? (root.primary ? studio.text : studio.text)
                : studio.textMuted
        }

        Text {
            text: root.text
            font: root.font
            color: root.enabled ? studio.text : studio.textMuted
            verticalAlignment: Text.AlignVCenter
        }
    }

    HoverHandler { id: hover }
}
