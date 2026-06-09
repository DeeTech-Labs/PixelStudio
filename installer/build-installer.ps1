# Build Release + windeployqt + prune staging + Inno Setup.
param(
    [string]$BuildDir = "",
    [string]$QtDir = "C:\Qt6\6.8.2\msvc2022_64",
    [string]$IsccPath = "",
    [switch]$SkipBuild,
    [switch]$SkipDeploy,
    [switch]$SkipPrune
)
$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$Root = Split-Path $PSScriptRoot -Parent
if (-not $BuildDir) { $BuildDir = Join-Path $Root "build\Release" }
$StageDir = Join-Path $PSScriptRoot "staging"
$OutputDir = Join-Path $PSScriptRoot "output"
$Iss = Join-Path $PSScriptRoot "PixelStudio.iss"
$QmlDir = Join-Path $Root "src\qml"

. (Join-Path $PSScriptRoot "deploy-prune.ps1")

function Get-AppVersion {
    $cmake = Join-Path $Root "cmake\PixelStudioVersion.cmake"
    $t = Get-Content $cmake -Raw
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

function Invoke-WinDeployQt {
    param(
        [Parameter(Mandatory)][string]$QtDir,
        [Parameter(Mandatory)][string]$ExePath,
        [Parameter(Mandatory)][string]$QmlSourceDir
    )

    $windeploy = Join-Path $QtDir "bin\windeployqt.exe"
    if (-not (Test-Path $windeploy)) { throw "windeployqt not found: $windeploy" }

    # --no-translations / --skip-plugin-types: Qt docs + community practice for smaller deploy trees.
    # --compiler-runtime: MSVC redist DLLs for machines without VS installed.
    & $windeploy `
        --release `
        --compiler-runtime `
        --qmldir $QmlSourceDir `
        --no-translations `
        --skip-plugin-types qmltooling `
        $ExePath
    if ($LASTEXITCODE -ne 0) { throw "windeployqt failed with exit code $LASTEXITCODE" }
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

$exe = Join-Path $BuildDir "appPixelStudio.exe"
if (-not (Test-Path $exe)) { throw "Missing $exe" }

if (-not $SkipDeploy) {
    Invoke-WinDeployQt -QtDir $QtDir -ExePath $exe -QmlSourceDir $QmlDir
}

Copy-DeployTreeToStage -SourceDir $BuildDir -StageDir $StageDir
Remove-DeployBuildArtifacts -StageDir $StageDir
Invoke-DeployPrune -StageDir $StageDir -SkipPrune:$SkipPrune
Remove-DeployEmptyDirs -RootDir $StageDir

$staged = (Get-ChildItem $StageDir -Recurse -File).Count
$stagedMb = [math]::Round(((Get-ChildItem $StageDir -Recurse -File | Measure-Object Length -Sum).Sum / 1MB), 2)
Write-Host "Installer payload: $staged file(s), $stagedMb MB"

$iscc = Resolve-Iscc $IsccPath
New-Item -ItemType Directory -Force -Path $OutputDir | Out-Null
& $iscc "/DStageDir=$StageDir" "/DMyAppVersion=$($ver.Display)" "/DMyAppVersionInfo=$($ver.Info)" $Iss
if ($LASTEXITCODE -ne 0) { throw "ISCC failed" }

$setup = Get-ChildItem $OutputDir -Filter "PixelStudio-Setup-*.exe" | Sort-Object LastWriteTime -Descending | Select-Object -First 1
if (-not $setup) { throw "Installer not produced in $OutputDir" }
Write-Host "Installer: $($setup.FullName)"
