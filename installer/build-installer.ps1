# Build Release + windeployqt + stage (fixed file list below) + Inno Setup.
param(
    [string]$BuildDir = "",
    [string]$QtDir = "C:\Qt6\6.8.2\msvc2022_64",
    [string]$IsccPath = "",
    [switch]$SkipBuild,
    [switch]$SkipDeploy
)
$ErrorActionPreference = "Stop"
$Root = Split-Path $PSScriptRoot -Parent
if (-not $BuildDir) { $BuildDir = Join-Path $Root "build\Release" }
$StageDir = Join-Path $PSScriptRoot "staging"
$OutputDir = Join-Path $PSScriptRoot "output"
$Iss = Join-Path $PSScriptRoot "PixelStudio.iss"

function Get-InstallerRuntimeFiles {
    return @(
        'appPixelStudio.exe'
        'iconengines/qsvgicon.dll'
        'imageformats/qgif.dll'
        'imageformats/qico.dll'
        'imageformats/qjpeg.dll'
        'imageformats/qsvg.dll'
        'PixelStudio/qmldir'
        'PixelStudio/src/qml/controls/qmldir'
        'PixelStudio/src/qml/controls/StudioButton.qml'
        'PixelStudio/src/qml/controls/StudioCheck.qml'
        'PixelStudio/src/qml/controls/StudioCombo.qml'
        'PixelStudio/src/qml/controls/StudioField.qml'
        'PixelStudio/src/qml/controls/StudioIcon.qml'
        'PixelStudio/src/qml/controls/StudioIconButton.qml'
        'PixelStudio/src/qml/controls/StudioNavIcon.qml'
        'PixelStudio/src/qml/controls/StudioScroll.qml'
        'PixelStudio/src/qml/controls/StudioSection.qml'
        'PixelStudio/src/qml/controls/StudioSegmented.qml'
        'PixelStudio/src/qml/controls/StudioSlider.qml'
        'PixelStudio/src/qml/controls/StudioSpin.qml'
        'PixelStudio/src/qml/controls/StudioTextField.qml'
        'PixelStudio/src/qml/I18n.qml'
        'PixelStudio/src/qml/Main.qml'
        'PixelStudio/src/qml/panels/ExportActionsPanel.qml'
        'PixelStudio/src/qml/panels/InspectorDock.qml'
        'PixelStudio/src/qml/panels/InspectorPageDisplay.qml'
        'PixelStudio/src/qml/panels/InspectorPageExport.qml'
        'PixelStudio/src/qml/panels/InspectorPageImage.qml'
        'PixelStudio/src/qml/panels/qmldir'
        'PixelStudio/src/qml/qmldir'
        'PixelStudio/src/qml/shell/CodeDock.qml'
        'PixelStudio/src/qml/shell/qmldir'
        'PixelStudio/src/qml/shell/StudioTabBar.qml'
        'PixelStudio/src/qml/shell/StudioTabChip.qml'
        'PixelStudio/src/qml/studio/qmldir'
        'PixelStudio/src/qml/studio/StudioFrame.qml'
        'PixelStudio/src/qml/studio/StudioViewport.qml'
        'PixelStudio/src/qml/Theme.qml'
        'PixelStudio/src/qml/welcome/qmldir'
        'PixelStudio/src/qml/welcome/WelcomeBackdrop.qml'
        'PixelStudio/src/qml/welcome/WelcomeDisplayViz.qml'
        'PixelStudio/src/qml/welcome/WelcomeRecentCard.qml'
        'PixelStudio/src/qml/welcome/WelcomeScreen.qml'
        'PixelStudio/src/qml/workspace/qmldir'
        'PixelStudio/src/qml/workspace/StudioDropCanvas.qml'
        'PixelStudio/src/qml/workspace/StudioExportHub.qml'
        'PixelStudio/src/qml/workspace/StudioWorkspace.qml'
        'platforms/qwindows.dll'
        'qml/Qt5Compat/GraphicalEffects/Blend.qml'
        'qml/Qt5Compat/GraphicalEffects/BrightnessContrast.qml'
        'qml/Qt5Compat/GraphicalEffects/Colorize.qml'
        'qml/Qt5Compat/GraphicalEffects/ColorOverlay.qml'
        'qml/Qt5Compat/GraphicalEffects/ConicalGradient.qml'
        'qml/Qt5Compat/GraphicalEffects/Desaturate.qml'
        'qml/Qt5Compat/GraphicalEffects/DirectionalBlur.qml'
        'qml/Qt5Compat/GraphicalEffects/Displace.qml'
        'qml/Qt5Compat/GraphicalEffects/DropShadow.qml'
        'qml/Qt5Compat/GraphicalEffects/FastBlur.qml'
        'qml/Qt5Compat/GraphicalEffects/GammaAdjust.qml'
        'qml/Qt5Compat/GraphicalEffects/GaussianBlur.qml'
        'qml/Qt5Compat/GraphicalEffects/Glow.qml'
        'qml/Qt5Compat/GraphicalEffects/HueSaturation.qml'
        'qml/Qt5Compat/GraphicalEffects/InnerShadow.qml'
        'qml/Qt5Compat/GraphicalEffects/LevelAdjust.qml'
        'qml/Qt5Compat/GraphicalEffects/LinearGradient.qml'
        'qml/Qt5Compat/GraphicalEffects/MaskedBlur.qml'
        'qml/Qt5Compat/GraphicalEffects/OpacityMask.qml'
        'qml/Qt5Compat/GraphicalEffects/plugins.qmltypes'
        'qml/Qt5Compat/GraphicalEffects/private/DropShadowBase.qml'
        'qml/Qt5Compat/GraphicalEffects/private/FastGlow.qml'
        'qml/Qt5Compat/GraphicalEffects/private/FastInnerShadow.qml'
        'qml/Qt5Compat/GraphicalEffects/private/GaussianDirectionalBlur.qml'
        'qml/Qt5Compat/GraphicalEffects/private/GaussianGlow.qml'
        'qml/Qt5Compat/GraphicalEffects/private/GaussianInnerShadow.qml'
        'qml/Qt5Compat/GraphicalEffects/private/GaussianMaskedBlur.qml'
        'qml/Qt5Compat/GraphicalEffects/private/plugins.qmltypes'
        'qml/Qt5Compat/GraphicalEffects/private/qmldir'
        'qml/Qt5Compat/GraphicalEffects/private/qtgraphicaleffectsprivateplugin.dll'
        'qml/Qt5Compat/GraphicalEffects/qmldir'
        'qml/Qt5Compat/GraphicalEffects/qtgraphicaleffectsplugin.dll'
        'qml/Qt5Compat/GraphicalEffects/RadialBlur.qml'
        'qml/Qt5Compat/GraphicalEffects/RadialGradient.qml'
        'qml/Qt5Compat/GraphicalEffects/RectangularGlow.qml'
        'qml/Qt5Compat/GraphicalEffects/RecursiveBlur.qml'
        'qml/Qt5Compat/GraphicalEffects/ThresholdMask.qml'
        'qml/Qt5Compat/GraphicalEffects/ZoomBlur.qml'
        'qml/QtQml/Models/modelsplugin.dll'
        'qml/QtQml/Models/plugins.qmltypes'
        'qml/QtQml/Models/qmldir'
        'qml/QtQml/plugins.qmltypes'
        'qml/QtQml/qmldir'
        'qml/QtQml/qmlplugin.dll'
        'qml/QtQml/WorkerScript/plugins.qmltypes'
        'qml/QtQml/WorkerScript/qmldir'
        'qml/QtQml/WorkerScript/workerscriptplugin.dll'
        'qml/QtQuick/Controls/Basic/AbstractButton.qml'
        'qml/QtQuick/Controls/Basic/Action.qml'
        'qml/QtQuick/Controls/Basic/ActionGroup.qml'
        'qml/QtQuick/Controls/Basic/ApplicationWindow.qml'
        'qml/QtQuick/Controls/Basic/BusyIndicator.qml'
        'qml/QtQuick/Controls/Basic/Button.qml'
        'qml/QtQuick/Controls/Basic/ButtonGroup.qml'
        'qml/QtQuick/Controls/Basic/Calendar.qml'
        'qml/QtQuick/Controls/Basic/CalendarModel.qml'
        'qml/QtQuick/Controls/Basic/CheckBox.qml'
        'qml/QtQuick/Controls/Basic/CheckDelegate.qml'
        'qml/QtQuick/Controls/Basic/ComboBox.qml'
        'qml/QtQuick/Controls/Basic/Container.qml'
        'qml/QtQuick/Controls/Basic/Control.qml'
        'qml/QtQuick/Controls/Basic/DayOfWeekRow.qml'
        'qml/QtQuick/Controls/Basic/DelayButton.qml'
        'qml/QtQuick/Controls/Basic/Dial.qml'
        'qml/QtQuick/Controls/Basic/Dialog.qml'
        'qml/QtQuick/Controls/Basic/DialogButtonBox.qml'
        'qml/QtQuick/Controls/Basic/Drawer.qml'
        'qml/QtQuick/Controls/Basic/Frame.qml'
        'qml/QtQuick/Controls/Basic/GroupBox.qml'
        'qml/QtQuick/Controls/Basic/HorizontalHeaderView.qml'
        'qml/QtQuick/Controls/Basic/impl/plugins.qmltypes'
        'qml/QtQuick/Controls/Basic/impl/qmldir'
        'qml/QtQuick/Controls/Basic/impl/qtquickcontrols2basicstyleimplplugin.dll'
        'qml/QtQuick/Controls/Basic/ItemDelegate.qml'
        'qml/QtQuick/Controls/Basic/Label.qml'
        'qml/QtQuick/Controls/Basic/Menu.qml'
        'qml/QtQuick/Controls/Basic/MenuBar.qml'
        'qml/QtQuick/Controls/Basic/MenuBarItem.qml'
        'qml/QtQuick/Controls/Basic/MenuItem.qml'
        'qml/QtQuick/Controls/Basic/MenuSeparator.qml'
        'qml/QtQuick/Controls/Basic/MonthGrid.qml'
        'qml/QtQuick/Controls/Basic/Page.qml'
        'qml/QtQuick/Controls/Basic/PageIndicator.qml'
        'qml/QtQuick/Controls/Basic/Pane.qml'
        'qml/QtQuick/Controls/Basic/plugins.qmltypes'
        'qml/QtQuick/Controls/Basic/Popup.qml'
        'qml/QtQuick/Controls/Basic/ProgressBar.qml'
        'qml/QtQuick/Controls/Basic/qmldir'
        'qml/QtQuick/Controls/Basic/qtquickcontrols2basicstyleplugin.dll'
        'qml/QtQuick/Controls/Basic/RadioButton.qml'
        'qml/QtQuick/Controls/Basic/RadioDelegate.qml'
        'qml/QtQuick/Controls/Basic/RangeSlider.qml'
        'qml/QtQuick/Controls/Basic/RoundButton.qml'
        'qml/QtQuick/Controls/Basic/ScrollBar.qml'
        'qml/QtQuick/Controls/Basic/ScrollIndicator.qml'
        'qml/QtQuick/Controls/Basic/ScrollView.qml'
        'qml/QtQuick/Controls/Basic/SelectionRectangle.qml'
        'qml/QtQuick/Controls/Basic/Slider.qml'
        'qml/QtQuick/Controls/Basic/SpinBox.qml'
        'qml/QtQuick/Controls/Basic/SplitView.qml'
        'qml/QtQuick/Controls/Basic/StackView.qml'
        'qml/QtQuick/Controls/Basic/SwipeDelegate.qml'
        'qml/QtQuick/Controls/Basic/SwipeView.qml'
        'qml/QtQuick/Controls/Basic/Switch.qml'
        'qml/QtQuick/Controls/Basic/SwitchDelegate.qml'
        'qml/QtQuick/Controls/Basic/TabBar.qml'
        'qml/QtQuick/Controls/Basic/TabButton.qml'
        'qml/QtQuick/Controls/Basic/TextArea.qml'
        'qml/QtQuick/Controls/Basic/TextField.qml'
        'qml/QtQuick/Controls/Basic/ToolBar.qml'
        'qml/QtQuick/Controls/Basic/ToolButton.qml'
        'qml/QtQuick/Controls/Basic/ToolSeparator.qml'
        'qml/QtQuick/Controls/Basic/ToolTip.qml'
        'qml/QtQuick/Controls/Basic/TreeViewDelegate.qml'
        'qml/QtQuick/Controls/Basic/Tumbler.qml'
        'qml/QtQuick/Controls/Basic/VerticalHeaderView.qml'
        'qml/QtQuick/Controls/Basic/WeekNumberColumn.qml'
        'qml/QtQuick/Controls/Fusion/ApplicationWindow.qml'
        'qml/QtQuick/Controls/Fusion/BusyIndicator.qml'
        'qml/QtQuick/Controls/Fusion/Button.qml'
        'qml/QtQuick/Controls/Fusion/CheckBox.qml'
        'qml/QtQuick/Controls/Fusion/CheckDelegate.qml'
        'qml/QtQuick/Controls/Fusion/ComboBox.qml'
        'qml/QtQuick/Controls/Fusion/DelayButton.qml'
        'qml/QtQuick/Controls/Fusion/Dial.qml'
        'qml/QtQuick/Controls/Fusion/Dialog.qml'
        'qml/QtQuick/Controls/Fusion/DialogButtonBox.qml'
        'qml/QtQuick/Controls/Fusion/Drawer.qml'
        'qml/QtQuick/Controls/Fusion/Frame.qml'
        'qml/QtQuick/Controls/Fusion/GroupBox.qml'
        'qml/QtQuick/Controls/Fusion/HorizontalHeaderView.qml'
        'qml/QtQuick/Controls/Fusion/impl/ButtonPanel.qml'
        'qml/QtQuick/Controls/Fusion/impl/CheckIndicator.qml'
        'qml/QtQuick/Controls/Fusion/impl/plugins.qmltypes'
        'qml/QtQuick/Controls/Fusion/impl/qmldir'
        'qml/QtQuick/Controls/Fusion/impl/qtquickcontrols2fusionstyleimplplugin.dll'
        'qml/QtQuick/Controls/Fusion/impl/RadioIndicator.qml'
        'qml/QtQuick/Controls/Fusion/impl/SliderGroove.qml'
        'qml/QtQuick/Controls/Fusion/impl/SliderHandle.qml'
        'qml/QtQuick/Controls/Fusion/impl/SwitchIndicator.qml'
        'qml/QtQuick/Controls/Fusion/ItemDelegate.qml'
        'qml/QtQuick/Controls/Fusion/Label.qml'
        'qml/QtQuick/Controls/Fusion/Menu.qml'
        'qml/QtQuick/Controls/Fusion/MenuBar.qml'
        'qml/QtQuick/Controls/Fusion/MenuBarItem.qml'
        'qml/QtQuick/Controls/Fusion/MenuItem.qml'
        'qml/QtQuick/Controls/Fusion/MenuSeparator.qml'
        'qml/QtQuick/Controls/Fusion/Page.qml'
        'qml/QtQuick/Controls/Fusion/PageIndicator.qml'
        'qml/QtQuick/Controls/Fusion/Pane.qml'
        'qml/QtQuick/Controls/Fusion/plugins.qmltypes'
        'qml/QtQuick/Controls/Fusion/Popup.qml'
        'qml/QtQuick/Controls/Fusion/ProgressBar.qml'
        'qml/QtQuick/Controls/Fusion/qmldir'
        'qml/QtQuick/Controls/Fusion/qtquickcontrols2fusionstyleplugin.dll'
        'qml/QtQuick/Controls/Fusion/RadioButton.qml'
        'qml/QtQuick/Controls/Fusion/RadioDelegate.qml'
        'qml/QtQuick/Controls/Fusion/RangeSlider.qml'
        'qml/QtQuick/Controls/Fusion/RoundButton.qml'
        'qml/QtQuick/Controls/Fusion/ScrollBar.qml'
        'qml/QtQuick/Controls/Fusion/ScrollIndicator.qml'
        'qml/QtQuick/Controls/Fusion/ScrollView.qml'
        'qml/QtQuick/Controls/Fusion/SelectionRectangle.qml'
        'qml/QtQuick/Controls/Fusion/Slider.qml'
        'qml/QtQuick/Controls/Fusion/SpinBox.qml'
        'qml/QtQuick/Controls/Fusion/SplitView.qml'
        'qml/QtQuick/Controls/Fusion/SwipeDelegate.qml'
        'qml/QtQuick/Controls/Fusion/Switch.qml'
        'qml/QtQuick/Controls/Fusion/SwitchDelegate.qml'
        'qml/QtQuick/Controls/Fusion/TabBar.qml'
        'qml/QtQuick/Controls/Fusion/TabButton.qml'
        'qml/QtQuick/Controls/Fusion/TextArea.qml'
        'qml/QtQuick/Controls/Fusion/TextField.qml'
        'qml/QtQuick/Controls/Fusion/ToolBar.qml'
        'qml/QtQuick/Controls/Fusion/ToolButton.qml'
        'qml/QtQuick/Controls/Fusion/ToolSeparator.qml'
        'qml/QtQuick/Controls/Fusion/ToolTip.qml'
        'qml/QtQuick/Controls/Fusion/TreeViewDelegate.qml'
        'qml/QtQuick/Controls/Fusion/Tumbler.qml'
        'qml/QtQuick/Controls/Fusion/VerticalHeaderView.qml'
        'qml/QtQuick/Controls/impl/plugins.qmltypes'
        'qml/QtQuick/Controls/impl/qmldir'
        'qml/QtQuick/Controls/impl/qtquickcontrols2implplugin.dll'
        'qml/QtQuick/Controls/plugins.qmltypes'
        'qml/QtQuick/Controls/qmldir'
        'qml/QtQuick/Controls/qtquickcontrols2plugin.dll'
        'qml/QtQuick/Dialogs/plugins.qmltypes'
        'qml/QtQuick/Dialogs/qmldir'
        'qml/QtQuick/Dialogs/qtquickdialogsplugin.dll'
        'qml/QtQuick/Dialogs/quickimpl/plugins.qmltypes'
        'qml/QtQuick/Dialogs/quickimpl/qml/+Fusion/ColorDialog.qml'
        'qml/QtQuick/Dialogs/quickimpl/qml/+Fusion/FileDialog.qml'
        'qml/QtQuick/Dialogs/quickimpl/qml/+Fusion/FileDialogDelegate.qml'
        'qml/QtQuick/Dialogs/quickimpl/qml/+Fusion/FolderBreadcrumbBar.qml'
        'qml/QtQuick/Dialogs/quickimpl/qml/+Fusion/FolderDialog.qml'
        'qml/QtQuick/Dialogs/quickimpl/qml/+Fusion/FolderDialogDelegate.qml'
        'qml/QtQuick/Dialogs/quickimpl/qml/+Fusion/FontDialog.qml'
        'qml/QtQuick/Dialogs/quickimpl/qml/+Fusion/MessageDialog.qml'
        'qml/QtQuick/Dialogs/quickimpl/qml/+Imagine/ColorDialog.qml'
        'qml/QtQuick/Dialogs/quickimpl/qml/+Imagine/FileDialog.qml'
        'qml/QtQuick/Dialogs/quickimpl/qml/+Imagine/FileDialogDelegate.qml'
        'qml/QtQuick/Dialogs/quickimpl/qml/+Imagine/FolderBreadcrumbBar.qml'
        'qml/QtQuick/Dialogs/quickimpl/qml/+Imagine/FolderDialog.qml'
        'qml/QtQuick/Dialogs/quickimpl/qml/+Imagine/FolderDialogDelegate.qml'
        'qml/QtQuick/Dialogs/quickimpl/qml/+Imagine/FontDialog.qml'
        'qml/QtQuick/Dialogs/quickimpl/qml/+Imagine/MessageDialog.qml'
        'qml/QtQuick/Dialogs/quickimpl/qml/+Material/ColorDialog.qml'
        'qml/QtQuick/Dialogs/quickimpl/qml/+Material/FileDialog.qml'
        'qml/QtQuick/Dialogs/quickimpl/qml/+Material/FileDialogDelegate.qml'
        'qml/QtQuick/Dialogs/quickimpl/qml/+Material/FolderBreadcrumbBar.qml'
        'qml/QtQuick/Dialogs/quickimpl/qml/+Material/FolderDialog.qml'
        'qml/QtQuick/Dialogs/quickimpl/qml/+Material/FolderDialogDelegate.qml'
        'qml/QtQuick/Dialogs/quickimpl/qml/+Material/FontDialog.qml'
        'qml/QtQuick/Dialogs/quickimpl/qml/+Material/MessageDialog.qml'
        'qml/QtQuick/Dialogs/quickimpl/qml/+Universal/ColorDialog.qml'
        'qml/QtQuick/Dialogs/quickimpl/qml/+Universal/FileDialog.qml'
        'qml/QtQuick/Dialogs/quickimpl/qml/+Universal/FileDialogDelegate.qml'
        'qml/QtQuick/Dialogs/quickimpl/qml/+Universal/FolderBreadcrumbBar.qml'
        'qml/QtQuick/Dialogs/quickimpl/qml/+Universal/FolderDialog.qml'
        'qml/QtQuick/Dialogs/quickimpl/qml/+Universal/FolderDialogDelegate.qml'
        'qml/QtQuick/Dialogs/quickimpl/qml/+Universal/FontDialog.qml'
        'qml/QtQuick/Dialogs/quickimpl/qml/+Universal/MessageDialog.qml'
        'qml/QtQuick/Dialogs/quickimpl/qml/ColorDialog.qml'
        'qml/QtQuick/Dialogs/quickimpl/qml/ColorInputs.qml'
        'qml/QtQuick/Dialogs/quickimpl/qml/FileDialog.qml'
        'qml/QtQuick/Dialogs/quickimpl/qml/FileDialogDelegate.qml'
        'qml/QtQuick/Dialogs/quickimpl/qml/FileDialogDelegateLabel.qml'
        'qml/QtQuick/Dialogs/quickimpl/qml/FolderBreadcrumbBar.qml'
        'qml/QtQuick/Dialogs/quickimpl/qml/FolderDialog.qml'
        'qml/QtQuick/Dialogs/quickimpl/qml/FolderDialogDelegate.qml'
        'qml/QtQuick/Dialogs/quickimpl/qml/FolderDialogDelegateLabel.qml'
        'qml/QtQuick/Dialogs/quickimpl/qml/FontDialog.qml'
        'qml/QtQuick/Dialogs/quickimpl/qml/FontDialogContent.qml'
        'qml/QtQuick/Dialogs/quickimpl/qml/HueGradient.qml'
        'qml/QtQuick/Dialogs/quickimpl/qml/MessageDialog.qml'
        'qml/QtQuick/Dialogs/quickimpl/qml/PickerHandle.qml'
        'qml/QtQuick/Dialogs/quickimpl/qml/SaturationLightnessPicker.qml'
        'qml/QtQuick/Dialogs/quickimpl/qmldir'
        'qml/QtQuick/Dialogs/quickimpl/qtquickdialogs2quickimplplugin.dll'
        'qml/QtQuick/Layouts/plugins.qmltypes'
        'qml/QtQuick/Layouts/qmldir'
        'qml/QtQuick/Layouts/qquicklayoutsplugin.dll'
        'qml/QtQuick/plugins.qmltypes'
        'qml/QtQuick/qmldir'
        'qml/QtQuick/qtquick2plugin.dll'
        'qml/QtQuick/Templates/plugins.qmltypes'
        'qml/QtQuick/Templates/qmldir'
        'qml/QtQuick/Templates/qtquicktemplates2plugin.dll'
        'qml/QtQuick/Window/qmldir'
        'qml/QtQuick/Window/quickwindow.qmltypes'
        'qml/QtQuick/Window/quickwindowplugin.dll'
        'Qt6Core.dll'
        'Qt6Gui.dll'
        'Qt6Network.dll'
        'Qt6OpenGL.dll'
        'Qt6Qml.dll'
        'Qt6QmlMeta.dll'
        'Qt6QmlModels.dll'
        'Qt6QmlWorkerScript.dll'
        'Qt6Quick.dll'
        'Qt6QuickControls2.dll'
        'Qt6QuickControls2Basic.dll'
        'Qt6QuickControls2Fusion.dll'
        'Qt6QuickControls2FusionStyleImpl.dll'
        'Qt6QuickControls2Impl.dll'
        'Qt6QuickDialogs2.dll'
        'Qt6QuickDialogs2QuickImpl.dll'
        'Qt6QuickDialogs2Utils.dll'
        'Qt6QuickEffects.dll'
        'Qt6QuickLayouts.dll'
        'Qt6QuickTemplates2.dll'
        'Qt6ShaderTools.dll'
        'Qt6Svg.dll'
        'tls/qschannelbackend.dll'
    )
}

function Get-AppVersion {
    $cmake = Join-Path $Root "cmake\PixelStudioVersion.cmake"
    $t = Get-Content $cmake -Raw
    # Match only set(...) lines — ignore examples in comments (e.g. PS_VERSION_FIX "b").
    if ($t -notmatch 'set\(PS_VERSION_GLOBAL\s+(\d+)\)') { throw "PS_VERSION_GLOBAL not found in $cmake" }
    $g = [int]$Matches[1]
    if ($t -notmatch 'set\(PS_VERSION_MAJOR\s+(\d+)\)') { throw "PS_VERSION_MAJOR not found in $cmake" }
    $mj = [int]$Matches[1]
    if ($t -notmatch 'set\(PS_VERSION_MINOR\s+(\d+)\)') { throw "PS_VERSION_MINOR not found in $cmake" }
    $mn = [int]$Matches[1]
    if ($t -notmatch 'set\(PS_VERSION_FIX\s+"([^"]*)"\)') { throw "PS_VERSION_FIX not found in $cmake" }
    $fx = $Matches[1]
    $display = if ($fx) { "$g.$mj.$mn$fx" } else { "$g.$mj.$mn" }
    $info = "$g.$mj.$mn.0"
    return @{ Display = $display; Info = $info }
}

function Invoke-VcVars {
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    $vs = & $vswhere -latest -property installationPath 2>$null
    if (-not $vs) { throw "Visual Studio not found" }
    $vcvars = Join-Path $vs "VC\Auxiliary\Build\vcvars64.bat"
    cmd /c "`"$vcvars`" >nul && set" | ForEach-Object {
        if ($_ -match '^([^=]+)=(.*)$') { Set-Item -Path "env:$($Matches[1])" -Value $Matches[2] -EA SilentlyContinue }
    }
}

function Resolve-Iscc([string]$Explicit) {
    if ($Explicit -and (Test-Path $Explicit)) { return (Resolve-Path $Explicit).Path }
    $cmd = Get-Command iscc.exe -EA SilentlyContinue
    if ($cmd) { return $cmd.Source }
    $default = "${env:ProgramFiles(x86)}\Inno Setup 6\ISCC.exe"
    if (Test-Path $default) { return $default }
    throw "ISCC.exe not found. Install Inno Setup 6 or pass -IsccPath."
}

function Resolve-StageSource {
    param([string]$Rel, [string]$DeployDir, [string]$QtDir)
    $norm = $Rel.Replace('/', '\')
    $fromDeploy = Join-Path $DeployDir $norm
    if (Test-Path $fromDeploy) { return $fromDeploy }
    if (-not $QtDir) { return $null }
    $qt = $QtDir.TrimEnd('\')
    if ($norm -match '^(imageformats|iconengines|platforms|tls)\\') {
        $fromPlugins = Join-Path $qt "plugins\$norm"
        if (Test-Path $fromPlugins) { return $fromPlugins }
    }
    if ($norm.StartsWith('qml\')) {
        $fromQml = Join-Path $qt $norm
        if (Test-Path $fromQml) { return $fromQml }
    }
    return $null
}

function Invoke-StageInstallerFiles {
    param([string]$DeployDir, [string]$StageDir, [string]$QtDir = "")
    $paths = Get-InstallerRuntimeFiles
    if (-not (Test-Path (Join-Path $DeployDir "appPixelStudio.exe"))) {
        throw "Deploy tree not found: $DeployDir"
    }
    if (Test-Path $StageDir) { Remove-Item -Recurse -Force $StageDir }
    New-Item -ItemType Directory -Force -Path $StageDir | Out-Null
    $missing = [System.Collections.Generic.List[string]]::new()
    $fromQt = 0
    foreach ($rel in $paths) {
        $norm = $rel.Replace('/', '\')
        $src = Resolve-StageSource -Rel $rel -DeployDir $DeployDir -QtDir $QtDir
        $dst = Join-Path $StageDir $norm
        if (-not $src) { $missing.Add($rel); continue }
        if (-not (Test-Path (Join-Path $DeployDir $norm))) { $fromQt++ }
        $parent = Split-Path $dst -Parent
        if (-not (Test-Path $parent)) { New-Item -ItemType Directory -Force -Path $parent | Out-Null }
        Copy-Item -LiteralPath $src -Destination $dst -Force
    }
    if ($missing.Count -gt 0) {
        throw "Missing $($missing.Count) file(s) after windeployqt (and Qt kit fallback). First: $($missing[0]). CI needs Qt module qt5compat if under qml/Qt5Compat/."
    }
    if ($fromQt -gt 0) {
        Write-Host "Staged $fromQt file(s) from Qt kit (not in deploy dir) — re-run windeployqt after installing qt5compat."
    }
    $staged = (Get-ChildItem $StageDir -Recurse -File).Count
    if ($staged -ne $paths.Count) {
        throw "Staging mismatch: expected $($paths.Count), got $staged"
    }
    Write-Host "Staged $staged files -> $StageDir"
}

$ver = Get-AppVersion
Write-Host "Version: $($ver.Display)"

if (-not $SkipBuild) {
    Invoke-VcVars
    $qt = $QtDir.TrimEnd('\')
    if (-not (Test-Path "$qt\bin\windeployqt.exe")) { throw "Qt not found: $qt" }
    if (-not (Test-Path (Join-Path $BuildDir "build.ninja"))) {
        & cmake -S $Root -B $BuildDir -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="$qt"
        if ($LASTEXITCODE -ne 0) { throw "cmake configure failed" }
    }
    & cmake --build $BuildDir
    if ($LASTEXITCODE -ne 0) { throw "cmake build failed" }
}

if (-not $SkipDeploy) {
    $exe = Join-Path $BuildDir "appPixelStudio.exe"
    if (-not (Test-Path $exe)) { throw "Missing $exe" }
    $windeploy = Join-Path $QtDir "bin\windeployqt.exe"
    $qmlDir = Join-Path $Root "src\qml"
    & $windeploy --release --compiler-runtime --qmldir $qmlDir $exe
    if ($LASTEXITCODE -ne 0) { throw "windeployqt failed" }
}

Invoke-StageInstallerFiles -DeployDir $BuildDir -StageDir $StageDir -QtDir $QtDir

$iscc = Resolve-Iscc $IsccPath
New-Item -ItemType Directory -Force -Path $OutputDir | Out-Null
& $iscc "/DStageDir=$StageDir" "/DMyAppVersion=$($ver.Display)" "/DMyAppVersionInfo=$($ver.Info)" $Iss
if ($LASTEXITCODE -ne 0) { throw "ISCC failed" }

$setup = Get-ChildItem $OutputDir -Filter "PixelStudio-Setup-*.exe" | Sort-Object LastWriteTime -Descending | Select-Object -First 1
if (-not $setup) { throw "Installer not produced in $OutputDir" }
Write-Host "Installer: $($setup.FullName)"