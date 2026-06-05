import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

pragma Translator: PixelStudio

Item {
    id: root
    default property alias content: body.data
    required property var studio
    property string title: ""
    property string hint: ""
    property bool collapsible: false
    property bool expanded: true

    implicitHeight: column.implicitHeight
    Layout.fillWidth: true
    width: parent ? parent.width : implicitWidth

    ColumnLayout {
        id: column
        width: root.width
        spacing: studio.spacingMd

        Rectangle {
            id: headerBox
            Layout.fillWidth: true
            Layout.preferredHeight: root.collapsible ? studio.controlHeight : titleRow.implicitHeight
            visible: root.title.length > 0
            radius: studio.radiusMd
            color: headerMa.containsMouse && root.collapsible
                ? studio.surfaceHover
                : (root.collapsible ? studio.surfaceInset : "transparent")
            border.width: root.collapsible ? 1 : 0
            border.color: root.collapsible
                ? (root.expanded ? studio.borderFocus : studio.border)
                : "transparent"

            RowLayout {
                id: titleRow
                anchors.fill: parent
                anchors.leftMargin: root.collapsible ? studio.spacingMd : 0
                anchors.rightMargin: root.collapsible ? studio.spacingMd : 0
                spacing: studio.spacingSm

                Label {
                    visible: root.collapsible
                    Layout.preferredWidth: 16
                    horizontalAlignment: Text.AlignHCenter
                    text: root.expanded ? "▾" : "▸"
                    font.pixelSize: studio.fontSizeSm
                    font.weight: Font.DemiBold
                    color: studio.text
                }
                Label {
                    Layout.fillWidth: true
                    text: root.title.toUpperCase()
                    font.family: studio.fontFamily
                    font.pixelSize: studio.fontSizeXs
                    font.weight: Font.DemiBold
                    color: studio.sectionLabel
                    verticalAlignment: Text.AlignVCenter
                }
            }

            MouseArea {
                id: headerMa
                anchors.fill: parent
                enabled: root.collapsible
                hoverEnabled: root.collapsible
                cursorShape: root.collapsible ? Qt.PointingHandCursor : Qt.ArrowCursor
                onClicked: root.expanded = !root.expanded
            }
        }

        Label {
            Layout.fillWidth: true
            visible: root.hint.length > 0 && root.expanded
            wrapMode: Text.WordWrap
            text: root.hint
            font.family: studio.fontFamily
            font.pixelSize: studio.fontSizeSm
            lineHeight: 1.35
            color: studio.textSecondary
        }

        ColumnLayout {
            id: body
            Layout.fillWidth: true
            visible: root.expanded
            spacing: studio.spacingMd
        }
    }
}
