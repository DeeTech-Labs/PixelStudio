# Parses cmake/PixelStudioVersion.cmake — single reader for PowerShell tooling.
Set-StrictMode -Version Latest

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
    if ($t -notmatch 'set\(PS_VERSION_FIX\s+"([^"]*)"\)') { throw "PS_VERSION_FIX not found in $cmake" }
    $fix = $Matches[1]

    $display = if ($fix) { "$major.$minor.$patch$fix" } else { "$major.$minor.$patch" }
    $info = "$major.$minor.$patch.0"

    return [PSCustomObject]@{
        Major   = $major
        Minor   = $minor
        Patch   = $patch
        Fix     = $fix
        Display = $display
        Info    = $info
        Tag     = "v$display"
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

    $ver = Read-PixelStudioVersion -RepoRoot $RepoRoot
    if ($Tag -ne $ver.Tag) {
        throw "Git tag '$Tag' does not match cmake version '$($ver.Tag)' (cmake/PixelStudioVersion.cmake)."
    }
    return $ver
}

# Прямой запуск: .\cmake\ReadPixelStudioVersion.ps1
# Dot-source (без вывода): . .\cmake\ReadPixelStudioVersion.ps1
if ($MyInvocation.InvocationName -ne '.') {
    Read-PixelStudioVersion | Format-List Display, Tag, Info, Major, Minor, Patch, Fix
}
