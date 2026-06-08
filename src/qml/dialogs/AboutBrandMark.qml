import QtQuick

pragma Translator: PixelStudio

Item {
    id: root

    property int markSize: 64

    implicitWidth: markSize
    implicitHeight: markSize

    Image {
        anchors.fill: parent
        source: "qrc:/ico/PixelStudio.png"
        sourceSize: Qt.size(256, 256)
        fillMode: Image.PreserveAspectFit
        smooth: false
        mipmap: true
    }
}
