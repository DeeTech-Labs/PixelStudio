import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import PixelStudio

pragma Translator: Shell


Rectangle {
    id: root

    required property var studio

    signal newProjectRequested()
    signal openRequested()
    signal saveRequested()
    signal toggleGridRequested()
    signal viewDualRequested()
    signal viewSourceRequested()
    signal viewOutputRequested()

    implicitHeight: studio.toolbarHeight
    color: studio.surface
    border.width: 1
    border.color: studio.border

    readonly property var items: [
        { icon: "plus", tip: qsTr("New"), action: "new", enabled: true },
        { icon: "folder-open", tip: qsTr("Open"), action: "open", enabled: true },
        { icon: "save", tip: qsTr("Save"), action: "save", enabled: true },
        { sep: true, enabled: false, icon: "", tip: "", action: "" },
        { icon: "rotate-cw", tip: "", action: "", enabled: false },
        { icon: "rotate-cw", tip: "", action: "", enabled: false },
        { sep: true, enabled: false, icon: "", tip: "", action: "" },
        { icon: "display", tip: "", action: "", enabled: false },
        { icon: "sparkles", tip: "", action: "", enabled: false },
        { icon: "upload", tip: "", action: "", enabled: false },
        { icon: "upload", tip: "", action: "", enabled: false },
        { icon: "eraser", tip: "", action: "", enabled: false },
        { icon: "palette", tip: "", action: "", enabled: false },
        { icon: "palette", tip: "", action: "", enabled: false },
        { sep: true, enabled: false, icon: "", tip: "", action: "" },
        { icon: "layout-grid", tip: qsTr("Toggle grid"), action: "grid", enabled: true },
        { icon: "columns-2", tip: qsTr("Dual view"), action: "dual", enabled: true },
        { icon: "image", tip: qsTr("Source"), action: "source", enabled: true },
        { icon: "file-image", tip: qsTr("Output"), action: "output", enabled: true },
        { icon: "display", tip: "", action: "", enabled: false },
        { icon: "columns-2", tip: "", action: "", enabled: false }
    ]

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: studio.spacingSm
        anchors.rightMargin: studio.spacingSm
        spacing: studio.spacingXs

        Repeater {
            model: root.items
            delegate: Item {
                required property var modelData
                Layout.preferredWidth: modelData.sep ? 8 : 28
                Layout.preferredHeight: 28

                Rectangle {
                    visible: modelData.sep === true
                    anchors.centerIn: parent
                    width: 1
                    height: 20
                    color: studio.border
                }

                readonly property bool gridActive: modelData.action === "grid" && appSettings.showPixelGrid

                ToolButton {
                    visible: modelData.sep !== true
                    anchors.fill: parent
                    enabled: modelData.enabled === true
                    ToolTip.visible: hovered && modelData.tip && modelData.tip.length > 0
                    ToolTip.text: modelData.tip || ""
                    onClicked: {
                        switch (modelData.action) {
                        case "new": root.newProjectRequested(); break
                        case "open": root.openRequested(); break
                        case "save": root.saveRequested(); break
                        case "grid": root.toggleGridRequested(); break
                        case "dual": root.viewDualRequested(); break
                        case "source": root.viewSourceRequested(); break
                        case "output": root.viewOutputRequested(); break
                        }
                    }
                    background: Rectangle {
                        radius: studio.radiusSm
                        color: gridActive
                            ? studio.accentSoft
                            : (parent.hovered && parent.enabled ? studio.railHover : "transparent")
                        border.width: gridActive || (parent.enabled && parent.hovered) ? 1 : 0
                        border.color: gridActive ? studio.accent : studio.accent
                    }
                    contentItem: StudioIcon {
                        anchors.centerIn: parent
                        name: modelData.icon || "plus"
                        iconSize: 14
                        tint: modelData.enabled
                            ? (gridActive || parent.hovered ? studio.accent : studio.textSecondary)
                            : studio.decorativeDisabled
                    }
                }
            }
        }

        Item { Layout.fillWidth: true }

        Rectangle {
            implicitWidth: zoomLabel.implicitWidth + studio.spacingSm * 2
            implicitHeight: 22
            color: studio.surfaceInput
            border.width: 1
            border.color: studio.border
            Label {
                id: zoomLabel
                anchors.centerIn: parent
                text: "200%"
                font.family: studio.fontFamilyMono
                font.pixelSize: studio.fontSizeXs
                color: studio.textMuted
            }
        }
    }
}
