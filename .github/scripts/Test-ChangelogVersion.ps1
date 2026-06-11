# Validates cmake version and CHANGELOG consistency on pull requests.
Set-StrictMode -Version Latest

function Get-VersionDisplayFromCmakeText {
    param([string]$Text)

    if ($Text -notmatch 'set\(PS_VERSION_MAJOR\s+(\d+)\)') { throw "PS_VERSION_MAJOR not found" }
    $major = [int]$Matches[1]
    if ($Text -notmatch 'set\(PS_VERSION_MINOR\s+(\d+)\)') { throw "PS_VERSION_MINOR not found" }
    $minor = [int]$Matches[1]
    if ($Text -notmatch 'set\(PS_VERSION_PATCH\s+(\d+)\)') { throw "PS_VERSION_PATCH not found" }
    $patch = [int]$Matches[1]

    if ($Text -match 'set\(PS_VERSION_PRERELEASE\s+"([^"]*)"\)') {
        $prerelease = $Matches[1]
        if ($prerelease) {
            Test-SemVerPrerelease -Prerelease $prerelease
            return "$major.$minor.$patch-$prerelease"
        }
        return "$major.$minor.$patch"
    }

    if ($Text -match 'set\(PS_VERSION_FIX\s+"([^"]*)"\)') {
        $fix = $Matches[1]
        if ($fix) { return "$major.$minor.$patch$fix" }
        return "$major.$minor.$patch"
    }

    throw "PS_VERSION_PRERELEASE not found in cmake version file"
}

function Test-ChangelogVersion {
    param(
        [string]$RepoRoot = "",
        [Parameter(Mandatory)][string]$BaseSha
    )

    $ErrorActionPreference = "Stop"

    if (-not $RepoRoot) {
        $RepoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
    }

    . (Join-Path $RepoRoot "cmake\ReadPixelStudioVersion.ps1")

    $headVer = Read-PixelStudioVersion -RepoRoot $RepoRoot
    Write-Host "Head version: $($headVer.Display) ($($headVer.Tag))"

    $changelogPath = Join-Path $RepoRoot "CHANGELOG.md"
    if (-not (Test-Path -LiteralPath $changelogPath)) {
        throw "CHANGELOG.md not found"
    }
    $changelog = Get-Content -LiteralPath $changelogPath -Raw
    if ($changelog -notmatch '## \[Unreleased\]') {
        throw "CHANGELOG.md must contain a ## [Unreleased] section"
    }

    $changed = @(git -C $RepoRoot diff --name-only "$BaseSha...HEAD")
    $versionFile = "cmake/PixelStudioVersion.cmake"
    $changelogFile = "CHANGELOG.md"
    $versionChanged = $changed -contains $versionFile
    $changelogChanged = $changed -contains $changelogFile

    if ($versionChanged -and -not $changelogChanged) {
        throw "cmake/PixelStudioVersion.cmake changed but CHANGELOG.md was not updated"
    }

    $baseCmake = git -C $RepoRoot show "${BaseSha}:${versionFile}" 2>$null
    if (-not $baseCmake) {
        Write-Host "Base version file not found in $BaseSha — skipping bump checks"
        return
    }

    $baseDisplay = Get-VersionDisplayFromCmakeText -Text $baseCmake
    Write-Host "Base version: $baseDisplay"

    if ($headVer.Display -eq $baseDisplay) {
        Write-Host "Version unchanged — OK"
        return
    }

    $escapedDisplay = [regex]::Escape($headVer.Display)
    $hasVersionSection = $changelog -match "## \[$escapedDisplay\]"
    $hasUnreleasedEntries = $changelog -match '(?ms)## \[Unreleased\]\s*\r?\n\s*\r?\n(### |\- )'

    if (-not $hasVersionSection -and -not $hasUnreleasedEntries) {
        throw @"
Version bumped ($baseDisplay -> $($headVer.Display)) but CHANGELOG.md has no entries under [Unreleased] and no section [$($headVer.Display)].
Add release notes before merging.
"@
    }

    Write-Host "Version bump documented in CHANGELOG — OK"
}
