# Versioning

> **Русский:** [../versioning.md](../versioning.md)

## Format

**`Major.Minor.Patch`** or **`Major.Minor.Patch` + fix**, e.g. `0.1.0`, `0.1.0b`, `1.2.3b`.

| Display | CMake |
|---------|--------|
| `1.2.3b` | `PS_VERSION_MAJOR=1`, `PS_VERSION_MINOR=2`, `PS_VERSION_PATCH=3`, `PS_VERSION_FIX="b"` |

Source of truth: [`cmake/PixelStudioVersion.cmake`](../../cmake/PixelStudioVersion.cmake).  
Git tags: `v0.1.0`, `v1.2.3b`. Check: `.\cmake\ReadPixelStudioVersion.ps1`

## PR labels

| Label | Drafter bump | CMake |
|-------|--------------|--------|
| `patch` | Default patch | `PS_VERSION_PATCH++`, clear `FIX` |
| `minor` | Minor | `PS_VERSION_MINOR++`, `PATCH=0`, clear `FIX` |
| `major` | Major | `PS_VERSION_MAJOR++`, reset `MINOR`/`PATCH`, clear `FIX` |
| `version-fix` | Suffix only | set `PS_VERSION_FIX`; tag manually e.g. `v1.2.3b` |

## Release

1. Align `cmake/PixelStudioVersion.cmake`
2. `git tag v1.2.3` — must match cmake (`ReadPixelStudioVersion.ps1`)
3. Push tag → [release.yml](../../.github/workflows/release.yml) builds the installer
