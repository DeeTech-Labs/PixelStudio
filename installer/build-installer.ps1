# Build Release + windeployqt + stage deploy tree + Inno Setup.
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

function Test-DeployPathExcluded {
    param(
        [string]$RelativePath,
        [System.Collections.Generic.HashSet[string]]$ExcludeDirNames,
        [string[]]$ExcludeExtensions,
        [string[]]$ExcludeFileNames
    )
    $rel = $RelativePath.Replace('\', '/')
    $segments = $rel -split '/'
    foreach ($seg in $segments) {
        if ($ExcludeDirNames.Contains($seg)) { return $true }
    }
    $leaf = [IO.Path]::GetFileName($RelativePath)
    if ($ExcludeFileNames -contains $leaf) { return $true }
    $ext = [IO.Path]::GetExtension($RelativePath)
    if ($ExcludeExtensions -contains $ext) { return $true }
    return $false
}

function Invoke-StageInstallerFiles {
    param([string]$DeployDir, [string]$StageDir)
    if (-not (Test-Path (Join-Path $DeployDir "appPixelStudio.exe"))) {
        throw "Deploy tree not found: $DeployDir"
    }
    if (Test-Path $StageDir) { Remove-Item -Recurse -Force $StageDir }
    New-Item -ItemType Directory -Force -Path $StageDir | Out-Null

    # After windeployqt: copy the deploy tree, skip CMake/Ninja build artifacts.
    $excludeDirNames = [System.Collections.Generic.HashSet[string]]::new(
        [StringComparer]::OrdinalIgnoreCase
    )
    foreach ($d in @(
            'CMakeFiles', 'appPixelStudio_autogen', 'pixelstudio_core_autogen',
            '.cmake', '.rcc', '.qt', 'obj', 'Testing', 'qmltooling'
        )) {
        [void]$excludeDirNames.Add($d)
    }
    $excludeExtensions = @(
        '.obj', '.lib', '.pdb', '.exp', '.ilk', '.tlog', '.recipe',
        '.ninja', '.ninja_deps', '.ninja_log', '.cmake', '.txt', '.json',
        '.a', '.res', '.i', '.pch'
    )
    $excludeFileNames = @(
        'build.ninja', 'cmake_install.cmake', 'CMakeCache.txt',
        'compile_commands.json', 'pixelstudio_core.lib'
    )

    $deployRoot = (Resolve-Path $DeployDir).Path.TrimEnd('\')
    $staged = 0
    foreach ($file in Get-ChildItem -Path $DeployDir -Recurse -File -Force) {
        $rel = $file.FullName.Substring($deployRoot.Length).TrimStart('\')
        if (Test-DeployPathExcluded $rel $excludeDirNames $excludeExtensions $excludeFileNames) {
            continue
        }
        $dst = Join-Path $StageDir $rel
        $parent = Split-Path $dst -Parent
        if (-not (Test-Path $parent)) {
            New-Item -ItemType Directory -Force -Path $parent | Out-Null
        }
        Copy-Item -LiteralPath $file.FullName -Destination $dst -Force
        $staged++
    }
    if ($staged -lt 1) { throw "No files staged from $DeployDir" }
    Write-Host "Staged $staged files from windeployqt tree -> $StageDir"
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

$ver = Get-AppVersion
Write-Host "Version: $($ver.Display)"

if (-not $SkipBuild) {
    Invoke-VcVars
    $qt = $QtDir.TrimEnd('\')
    if (-not (Test-Path "$qt\bin\windeployqt.exe")) { throw "Qt not found: $qt" }
    if (-not (Test-Path (Join-Path $BuildDir "CMakeCache.txt"))) {
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

Invoke-StageInstallerFiles -DeployDir $BuildDir -StageDir $StageDir

$iscc = Resolve-Iscc $IsccPath
New-Item -ItemType Directory -Force -Path $OutputDir | Out-Null
& $iscc "/DStageDir=$StageDir" "/DMyAppVersion=$($ver.Display)" "/DMyAppVersionInfo=$($ver.Info)" $Iss
if ($LASTEXITCODE -ne 0) { throw "ISCC failed" }

$setup = Get-ChildItem $OutputDir -Filter "PixelStudio-Setup-*.exe" | Sort-Object LastWriteTime -Descending | Select-Object -First 1
if (-not $setup) { throw "Installer not produced in $OutputDir" }
Write-Host "Installer: $($setup.FullName)"
