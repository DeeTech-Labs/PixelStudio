import QtQuick

pragma Translator: PixelStudio

Item {
    id: root
    implicitWidth: 88
    implicitHeight: 88

    Image {
        anchors.centerIn: parent
        width: 88
        height: 88
        source: "qrc:/ico/PixelStudio.png"
        sourceSize: Qt.size(256, 256)
        fillMode: Image.PreserveAspectFit
        smooth: false
        mipmap: true
    }
}
