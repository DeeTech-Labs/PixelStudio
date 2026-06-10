# Deploy staging + prune helpers for PixelStudio installer pipeline.
# Dot-source from windows-build-installer.ps1.

Set-StrictMode -Version Latest

$script:AppExeName = 'PixelStudio.exe'

function Get-DeployBuildArtifactGlobs {
    return @(
        '*.obj'
        '*.pdb'
        '*.ilk'
        '*.exp'
        '*.lib'
        '*.tlog'
        '*.recipe'
        '*.res'
        '*.iobj'
        '*.ipdb'
        'build.ninja'
        '.ninja_deps'
        '.ninja_log'
        'compile_commands.json'
        'CMakeCache.txt'
        'cmake_install.cmake'
        'CTestTestfile.cmake'
        'install_manifest.txt'
        'rules.ninja'
        'build-*.ninja'
        'qrc_*.cpp'
        'moc_*.cpp'
        'moc_*.h'
        '*.qmlc'
        '*.jsc'
    )
}

function Get-DeployBuildArtifactDirs {
    return @(
        'CMakeFiles'
        '.qt'
        'PixelStudio_autogen'
        'pixelstudio_core_autogen'
        'Testing'
        '.cmake'
        'qmltooling'
        'translations'
    )
}

function Get-DeployPruneGroups {
    return @(
        @{
            Name = 'Unused Quick Controls styles (Imagine)'
            Paths = @(
                'qml\QtQuick\Controls\Imagine'
                'Qt6QuickControls2Imagine.dll'
                'Qt6QuickControls2ImagineStyleImpl.dll'
            )
        }
        @{
            Name = 'Unused Quick Controls styles (Universal)'
            Paths = @(
                'qml\QtQuick\Controls\Universal'
                'Qt6QuickControls2Universal.dll'
                'Qt6QuickControls2UniversalStyleImpl.dll'
            )
        }
        @{
            Name = 'Unused Quick Controls styles (Material)'
            Paths = @(
                'qml\QtQuick\Controls\Material'
                'Qt6QuickControls2Material.dll'
                'Qt6QuickControls2MaterialStyleImpl.dll'
            )
        }
        @{
            Name = 'Unused Quick Controls styles (FluentWinUI3)'
            Paths = @(
                'qml\QtQuick\Controls\FluentWinUI3'
                'Qt6QuickControls2FluentWinUI3.dll'
                'Qt6QuickControls2FluentWinUI3StyleImpl.dll'
            )
        }
        @{
            Name = 'Unused Quick Controls styles (Windows)'
            Paths = @(
                'qml\QtQuick\Controls\Windows'
                'Qt6QuickControls2WindowsStyleImpl.dll'
            )
        }
        @{
            Name = 'Dialog QML variants for unused styles'
            Paths = @(
                'qml\QtQuick\Dialogs\quickimpl\qml\+Imagine'
                'qml\QtQuick\Dialogs\quickimpl\qml\+Material'
                'qml\QtQuick\Dialogs\quickimpl\qml\+Universal'
            )
        }
        @{
            Name = 'Unused image format plugins'
            Paths = @(
                'imageformats\qicns.dll'
                'imageformats\qjp2.dll'
                'imageformats\qpdf.dll'
                'imageformats\qtga.dll'
                'imageformats\qwbmp.dll'
            )
        }
        @{
            Name = 'Qt Quick 3D (not used)'
            Paths = @(
                'Qt6Quick3D.dll'
                'Qt6Quick3DAssetImport.dll'
                'Qt6Quick3DEffects.dll'
                'Qt6Quick3DHelpers.dll'
                'Qt6Quick3DHelpersImpl.dll'
                'Qt6Quick3DParticleEffects.dll'
                'Qt6Quick3DParticles.dll'
                'Qt6Quick3DRuntimeRender.dll'
                'Qt6Quick3DUtils.dll'
                'qml\QtQuick3D'
            )
        }
        @{
            Name = 'Optional plugins.qmltypes metadata'
            Paths = @('**\plugins.qmltypes')
            Glob = $true
        }
        @{
            Name = 'Unused modules (PDF, virtual keyboard)'
            Paths = @(
                'Qt6Pdf.dll'
                'Qt6VirtualKeyboard.dll'
                'qml\QtQuick\Pdf'
            )
        }
        @{
            Name = 'Unused Qt Quick Shapes'
            Paths = @(
                'Qt6QuickShapes.dll'
                'qml\QtQuick\Shapes'
            )
        }
        @{
            Name = 'Unused Qt Quick NativeStyle'
            Paths = @('qml\QtQuick\NativeStyle')
        }
        @{
            Name = 'Build-only QML module metadata'
            Paths = @(
                'PixelStudio\PixelStudio.qmltypes'
                'PixelStudio\PixelStudio_qml_module_dir_map.qrc'
            )
        }
        @{
            Name = 'Unused touch input plugin'
            Paths = @('generic\qtuiotouchplugin.dll')
        }
    )
}

function Get-DeployRuntimeDirNames {
    return @(
        'platforms'
        'imageformats'
        'iconengines'
        'tls'
        'styles'
        'generic'
        'networkinformation'
        'qml'
        'PixelStudio'
    )
}

function Copy-DeployTreeToStage {
    param(
        [Parameter(Mandatory)][string]$SourceDir,
        [Parameter(Mandatory)][string]$StageDir
    )

    if (Test-Path $StageDir) {
        Remove-Item -LiteralPath $StageDir -Recurse -Force
    }
    New-Item -ItemType Directory -Force -Path $StageDir | Out-Null

    $exe = Join-Path $SourceDir $script:AppExeName
    if (-not (Test-Path -LiteralPath $exe)) {
        throw "Deploy source missing executable: $exe"
    }
    Copy-Item -LiteralPath $exe -Destination $StageDir -Force

    Get-ChildItem -LiteralPath $SourceDir -File -Filter '*.dll' -ErrorAction SilentlyContinue |
        Copy-Item -Destination $StageDir -Force

    $redist = Join-Path $SourceDir 'vc_redist.x64.exe'
    if (Test-Path -LiteralPath $redist) {
        Copy-Item -LiteralPath $redist -Destination $StageDir -Force
    }

    foreach ($dirName in Get-DeployRuntimeDirNames) {
        $srcDir = Join-Path $SourceDir $dirName
        if (-not (Test-Path -LiteralPath $srcDir)) { continue }
        $dstDir = Join-Path $StageDir $dirName
        Copy-Item -LiteralPath $srcDir -Destination $dstDir -Recurse -Force
    }

    $copied = (Get-ChildItem -LiteralPath $StageDir -Recurse -File).Count
    Write-Host "Staged deploy tree: $copied file(s) -> $StageDir"
}

function Remove-DeployBuildArtifacts {
    param([Parameter(Mandatory)][string]$StageDir)

    $removed = 0
    foreach ($dirName in Get-DeployBuildArtifactDirs) {
        $path = Join-Path $StageDir $dirName
        if (-not (Test-Path -LiteralPath $path)) { continue }
        Remove-Item -LiteralPath $path -Recurse -Force
        $removed++
        Write-Host "  removed build dir: $dirName"
    }

    foreach ($glob in Get-DeployBuildArtifactGlobs) {
        Get-ChildItem -LiteralPath $StageDir -Recurse -File -Filter $glob -ErrorAction SilentlyContinue |
            ForEach-Object {
                Remove-Item -LiteralPath $_.FullName -Force
                $removed++
            }
    }

    Write-Host "Removed $removed build artifact path(s) from staging"
}

function Resolve-DeployGroupPaths {
    param(
        [Parameter(Mandatory)][string]$StageDir,
        [Parameter(Mandatory)][hashtable]$Group
    )

    $resolved = [System.Collections.Generic.List[string]]::new()
    $useGlob = $Group.ContainsKey('Glob') -and $Group.Glob
    foreach ($rel in $Group.Paths) {
        if ($useGlob) {
            Get-ChildItem -LiteralPath $StageDir -Recurse -File -Filter ([IO.Path]::GetFileName($rel)) -ErrorAction SilentlyContinue |
                ForEach-Object { $resolved.Add($_.FullName) }
            continue
        }

        $full = Join-Path $StageDir ($rel.Replace('/', '\'))
        if (Test-Path -LiteralPath $full) {
            $resolved.Add($full)
        }
    }
    return $resolved
}

function Get-DeployPathsByteSize {
    param([Parameter(Mandatory)][string[]]$Paths)

    $sum = 0L
    foreach ($path in $Paths) {
        if (-not (Test-Path -LiteralPath $path)) { continue }
        $item = Get-Item -LiteralPath $path
        if ($item.PSIsContainer) {
            foreach ($file in (Get-ChildItem -LiteralPath $path -Recurse -File -ErrorAction SilentlyContinue)) {
                $sum += $file.Length
            }
        }
        else {
            $sum += $item.Length
        }
    }
    return $sum
}

function Remove-DeployPaths {
    param([Parameter(Mandatory)][string[]]$Paths)

    foreach ($path in $Paths) {
        if (-not (Test-Path -LiteralPath $path)) { continue }
        $item = Get-Item -LiteralPath $path
        if ($item.PSIsContainer) {
            Remove-Item -LiteralPath $path -Recurse -Force
        }
        else {
            Remove-Item -LiteralPath $path -Force
        }
    }
}

function Invoke-DeployPrune {
    param(
        [Parameter(Mandatory)][string]$StageDir,
        [switch]$SkipPrune
    )

    if ($SkipPrune) {
        Write-Host 'Prune skipped (-SkipPrune).'
        return
    }

    $removedGroups = 0
    $savedBytes = 0L

    foreach ($group in Get-DeployPruneGroups) {
        $paths = @(Resolve-DeployGroupPaths -StageDir $StageDir -Group $group)
        if ($paths.Count -eq 0) { continue }

        $bytesBefore = Get-DeployPathsByteSize -Paths $paths
        Remove-DeployPaths -Paths $paths

        $removedGroups++
        $savedBytes += $bytesBefore
        Write-Host ("  pruned: {0} ({1:N0} bytes)" -f $group.Name, $bytesBefore)
    }

    Write-Host ("Prune done: {0} group(s), ~{1:N2} MB removed." -f $removedGroups, ($savedBytes / 1MB))
}

function Remove-DeployEmptyDirs {
    param([Parameter(Mandatory)][string]$RootDir)

    $removed = 0
    do {
        $found = $false
        Get-ChildItem -LiteralPath $RootDir -Recurse -Directory -ErrorAction SilentlyContinue |
            Sort-Object { $_.FullName.Length } -Descending |
            ForEach-Object {
                if ((Get-ChildItem -LiteralPath $_.FullName -Force | Measure-Object).Count -eq 0) {
                    Remove-Item -LiteralPath $_.FullName -Force
                    $removed++
                    $found = $true
                }
            }
    } while ($found)

    if ($removed -gt 0) {
        Write-Host "Removed $removed empty director(ies)."
    }
}
