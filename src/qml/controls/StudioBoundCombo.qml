import QtQuick
import QtQuick.Controls

pragma Translator: Controls


Item {
    id: root

    required property var studio
    property var boundModel: []
    property string valueRole: "mode"
    property var boundValue: undefined
    property string textRole: "name"
    property string toolTipText: ""

    signal valueSelected(var value)

    implicitWidth: combo.implicitWidth
    implicitHeight: combo.implicitHeight

    StudioCombo {
        id: combo
        anchors.fill: parent
        studio: root.studio
        model: root.boundModel
        textRole: root.textRole
        toolTipText: root.toolTipText

        Component.onCompleted: root.syncIndex()

        onActivated: {
            const item = model[currentIndex]
            if (item && item[root.valueRole] !== undefined)
                root.valueSelected(item[root.valueRole])
        }
    }

    function syncIndex() {
        if (!combo.model || combo.model.length === undefined)
            return
        for (let i = 0; i < combo.model.length; ++i) {
            if (combo.model[i][valueRole] === boundValue) {
                combo.currentIndex = i
                return
            }
        }
    }

    onBoundValueChanged: syncIndex()
    onBoundModelChanged: syncIndex()
}
