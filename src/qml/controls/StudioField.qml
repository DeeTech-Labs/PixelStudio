import QtQuick
import QtQuick.Controls

pragma Translator: Controls


Label {
    required property var studio
    property string labelText: ""
    font.family: studio.fontFamily
    font.pixelSize: studio.fontSizeSm
    font.weight: Font.DemiBold
    color: studio.sectionLabel
    topPadding: studio.spacingXs
    text: labelText
}
