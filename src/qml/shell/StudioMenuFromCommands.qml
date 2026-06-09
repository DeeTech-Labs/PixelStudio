import QtQuick
import QtQuick.Controls
import PixelStudio

pragma Translator: Shell


Menu {
    id: root

    property var commands: []
    property bool useMnemonics: true
    property bool menuEnabled: true

    enabled: menuEnabled

    Component {
        id: sepComp
        MenuSeparator {}
    }

    Component {
        id: actComp
        Action {
            property var entry: null

            text: {
                if (!entry || !entry.label)
                    return ""
                const translated = WorkflowRouter.shellTr(entry.label)
                return root.useMnemonics
                    ? translated
                    : WorkflowRouter.stripMenuMnemonic(translated)
            }
            shortcut: entry && entry.shortcut ? entry.shortcut : ""
            enabled: entry && entry.command ? WorkflowRouter.commandEnabled(entry.command) : true
            checkable: entry && entry.checkable === true
            checked: checkable && entry ? WorkflowRouter.commandChecked(entry.command) : false

            onTriggered: {
                if (entry && entry.command)
                    WorkflowRouter.run(entry.command)
            }
        }
    }

    Instantiator {
        model: root.commands
        delegate: Loader {
            id: loader
            required property var modelData
            asynchronous: false
            sourceComponent: modelData.separator ? sepComp : actComp

            onLoaded: {
                if (item && !modelData.separator)
                    item.entry = modelData
            }
        }

        onObjectAdded: function(index, object) {
            const attach = () => {
                if (!object.item)
                    return
                if (object.modelData.separator)
                    root.insertItem(index, object.item)
                else
                    root.insertAction(index, object.item)
            }
            if (object.status === Loader.Ready)
                attach()
            else
                object.loaded.connect(attach)
        }

        onObjectRemoved: function(index, object) {
            if (!object.item)
                return
            if (object.modelData.separator)
                root.removeItem(object.item)
            else
                root.removeAction(object.item)
        }
    }
}
