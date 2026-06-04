import QtQuick

pragma Translator: PixelStudio

QtObject {
    id: root

    property color accent: "#3B9EFF"
    property color accentHover: "#5CADFF"
    property color accentPressed: "#2563EB"
    property color accentMuted: "#1E3A5F"
    property color accentOnAccent: "#FFFFFF"
    property color accentSoft: "#3B9EFF18"
    property color accentGlow: "#3B9EFF33"

    property color primary: accent
    property color primaryHover: accentHover
    property color primaryPressed: accentPressed
    property color primaryMuted: accentMuted

    property color background: "#1A1A1A"
    property color backgroundElevated: "#212121"
    property color canvas: "#141414"
    property color surface: "#2B2B2B"
    property color surfaceRaised: "#333333"
    property color surfaceHover: "#3D3D3D"
    property color surfaceOverlay: "#454545"
    property color surfaceInput: "#262626"
    property color surfaceInset: "#181818"

    property color text: "#E8E8E8"
    property color textSecondary: "#A3A3A3"
    property color textMuted: "#8A8A8A"
    property color sectionLabel: "#C8C8C8"

    property color border: "#404040"
    property color borderFocus: accentHover
    property color divider: "#2E2E2E"

    property color success: "#4ADE80"
    property color error: "#F87171"
    property color warning: "#FACC15"

    property color shadow: "#00000055"
    property color shadowMd: "#00000077"
    property color previewBackdrop: canvas
    property color floatingPanel: "#2B2B2BF0"
    property color checkerA: "#252525"
    property color checkerB: "#1C1C1C"
    property color railActive: "#3B9EFF22"
    property color railHover: "#FFFFFF0A"

    property int fontSizeXs: 11
    property int fontSizeSm: 12
    property int fontSizeBase: 13
    property int fontSizeLg: 14
    property int fontSizeXl: 15
    property int fontSize2xl: 18

    property string fontFamily: "'Segoe UI Variable', 'Segoe UI', system-ui, sans-serif"
    property string fontFamilyMono: "'Cascadia Code', Consolas, monospace"

    property int spacingXs: 4
    property int spacingSm: 6
    property int spacingMd: 10
    property int spacingLg: 14
    property int spacingXl: 18
    property int spacing2xl: 24

    property int radiusSm: 4
    property int radiusMd: 6
    property int radiusLg: 8
    property int radiusXl: 10
    property int radiusPill: 999

    property int headerHeight: 44
    property int statusHeight: 26
    property int inspectorWidth: 380
    property int inspectorNavWidth: 48
    property int codeDockHeight: 220
    property int scrollContentGutter: 8
    property int inspectorPadding: 16
    property int controlHeight: 32
    property int controlHeightSm: 26
    property int frameHeaderHeight: 36
    property int frameFooterHeight: 32

    property int scrollBarWidth: 5
    property int scrollBarRadius: 2
    property int splitHandleSize: 6

    property int durationFast: 80
    property int durationNormal: 140
    property int durationSlow: 220
}
