import QtQuick

pragma Translator: Shell


QtObject {
    id: root

    // Retro-pixel accent (teal)
    property color accent: "#40e0d0"
    property color accentHover: "#52edd9"
    property color accentPressed: "#1f9a88"
    property color accentMuted: "#40e0d044"
    property color accentOnAccent: "#0d1117"
    property color accentSoft: "#40e0d018"
    property color accentGlow: "#40e0d033"
    property color accentDark: "#1f9a88"
    property color accentTop: "#52edd9"
    property color accentBottom: "#36d4c4"

    property color primary: accent
    property color primaryHover: accentHover
    property color primaryPressed: accentPressed
    property color primaryMuted: accentMuted

    property color background: "#0d1117"
    property color backgroundElevated: "#0a0e14"
    property color canvas: "#0b0e14"
    property color surface: "#161b22"
    property color surfaceRaised: "#1c2129"
    property color surfaceHover: "#21262d"
    property color surfaceOverlay: "#262c36"
    property color surfaceInput: "#161b22"
    property color surfaceInset: "#0d1117"

    property color text: "#ffffff"
    property color textSecondary: "#8b949e"
    property color textMuted: "#6e7681"
    property color sectionLabel: "#8b949e"

    property color border: "#30363d"
    property color borderFocus: accent
    property color divider: "#21262d"

    property color success: "#3fb950"
    property color error: "#f85149"
    property color warning: "#d29922"

    property color shadow: "#00000066"
    property color shadowMd: "#00000088"
    property color previewBackdrop: "#0b0e14"
    property color floatingPanel: "#161b22f0"
    property color checkerA: "#1a1f26"
    property color checkerB: "#12161c"
    property color railActive: "#40e0d022"
    property color railHover: "#ffffff0a"
    property color decorativeDisabled: "#3d444d"

    property int fontSizeXs: 10
    property int fontSizeSm: 11
    property int fontSizeBase: 13
    property int fontSizeLg: 14
    property int fontSizeXl: 15
    property int fontSize2xl: 18
    property int fontSizePixel: 10
    property int fontSizePixelLg: 20

    property string fontFamily: "'Segoe UI Variable', 'Segoe UI', system-ui, sans-serif"
    property string fontFamilyMono: "'Cascadia Code', Consolas, monospace"
    property string fontFamilyPixel: "Press Start 2P"

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
    property int radiusPill: 10

    property int pixelBorderWidth: 1
    property int panelBracketSize: 4

    property int headerHeight: 44
    property int menuBarHeight: 28
    property int tabBarHeight: 32
    property int toolbarHeight: 32
    property int statusHeight: 24
    property int inspectorWidth: 300
    property int inspectorNavWidth: 0
    property int leftDockWidth: 220
    property int codeDockHeight: 200
    property int scrollContentGutter: 8
    property int inspectorPadding: 12
    property int controlHeight: 30
    property int controlHeightSm: 24
    property int frameHeaderHeight: 28
    property int frameFooterHeight: 28

    property int scrollBarWidth: 8
    property int scrollBarRadius: 4
    property int splitHandleSize: 4

    property int durationFast: 80
    property int durationNormal: 140
    property int durationSlow: 220
}
