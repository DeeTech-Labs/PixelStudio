# Parses cmake/PixelStudioVersion.cmake — single reader for PowerShell tooling.
Set-StrictMode -Version Latest

function Test-SemVerPrerelease {
    param(
        [Parameter(Mandatory)][string]$Prerelease
    )

    foreach ($id in $Prerelease.Split('.')) {
        if ($id -eq '') {
            throw "Invalid PS_VERSION_PRERELEASE '$Prerelease': empty identifier."
        }
        if ($id -match '[^0-9A-Za-z-]') {
            throw "Invalid PS_VERSION_PRERELEASE '$Prerelease': identifier '$id' must contain only [0-9A-Za-z-]."
        }
        if ($id -match '^\d+$' -and $id -match '^0\d') {
            throw "Invalid PS_VERSION_PRERELEASE '$Prerelease': numeric identifier '$id' must not have leading zeros."
        }
    }
}

function Read-PixelStudioVersion {
    param(
        [string]$RepoRoot = ""
    )

    if (-not $RepoRoot) {
        $RepoRoot = Split-Path $PSScriptRoot -Parent
    }

    $cmake = Join-Path $RepoRoot "cmake\PixelStudioVersion.cmake"
    if (-not (Test-Path -LiteralPath $cmake)) {
        throw "Version file not found: $cmake"
    }

    $t = Get-Content -LiteralPath $cmake -Raw
    if ($t -notmatch 'set\(PS_VERSION_MAJOR\s+(\d+)\)') { throw "PS_VERSION_MAJOR not found in $cmake" }
    $major = [int]$Matches[1]
    if ($t -notmatch 'set\(PS_VERSION_MINOR\s+(\d+)\)') { throw "PS_VERSION_MINOR not found in $cmake" }
    $minor = [int]$Matches[1]
    if ($t -notmatch 'set\(PS_VERSION_PATCH\s+(\d+)\)') { throw "PS_VERSION_PATCH not found in $cmake" }
    $patch = [int]$Matches[1]
    if ($t -notmatch 'set\(PS_VERSION_PRERELEASE\s+"([^"]*)"\)') { throw "PS_VERSION_PRERELEASE not found in $cmake" }
    $prerelease = $Matches[1]

    if ($prerelease) {
        Test-SemVerPrerelease -Prerelease $prerelease
    }

    $display = if ($prerelease) { "$major.$minor.$patch-$prerelease" } else { "$major.$minor.$patch" }
    $info = "$major.$minor.$patch.0"

    return [PSCustomObject]@{
        Major      = $major
        Minor      = $minor
        Patch      = $patch
        Prerelease = $prerelease
        Display    = $display
        Info       = $info
        Tag        = "v$display"
    }
}

function Write-PixelStudioInnoVersionInclude {
    param(
        [Parameter(Mandatory)][string]$OutputPath,
        [Parameter(Mandatory)][object]$Version
    )

    $parent = Split-Path $OutputPath -Parent
    if (-not (Test-Path $parent)) {
        New-Item -ItemType Directory -Force -Path $parent | Out-Null
    }

    $lines = @(
        "; Auto-generated - do not edit. Source: cmake/PixelStudioVersion.cmake"
        "#define MyAppVersion `"$($Version.Display)`""
        "#define MyAppVersionInfo `"$($Version.Info)`""
        ""
    )
    $utf8 = New-Object System.Text.UTF8Encoding $false
    [System.IO.File]::WriteAllText($OutputPath, ($lines -join [Environment]::NewLine) + [Environment]::NewLine, $utf8)
}

function Test-PixelStudioReleaseTag {
    param(
        [Parameter(Mandatory)][string]$Tag,
        [string]$RepoRoot = ""
    )

    if ($Tag -notmatch '^v\d+\.\d+\.\d+(-[0-9A-Za-z-]+(\.[0-9A-Za-z-]+)*)?$') {
        throw "Invalid SemVer tag '$Tag' (expected vX.Y.Z or vX.Y.Z-prerelease, e.g. v1.2.3 or v1.2.3-rc.1)."
    }

    $ver = Read-PixelStudioVersion -RepoRoot $RepoRoot
    if ($Tag -ne $ver.Tag) {
        throw "Git tag '$Tag' does not match cmake version '$($ver.Tag)' (cmake/PixelStudioVersion.cmake)."
    }
    return $ver
}

# Прямой запуск: .\cmake\ReadPixelStudioVersion.ps1
# Dot-source (без вывода): . .\cmake\ReadPixelStudioVersion.ps1
if ($MyInvocation.InvocationName -ne '.') {
    Read-PixelStudioVersion | Format-List Display, Tag, Info, Major, Minor, Patch, Prerelease
}
