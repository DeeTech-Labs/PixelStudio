# Versioning

> **Русский:** [../versioning.md](../versioning.md)

## Format

**`G.M.m`** or **`G.M.m` + fix**, e.g. `0.1.0`, `0.1.0b`, `1.2.3b`.

| Display | CMake |
|---------|--------|
| `1.2.3b` | `PS_VERSION_GLOBAL=1`, `PS_VERSION_MAJOR=2`, `PS_VERSION_MINOR=3`, `PS_VERSION_FIX="b"` |

Source of truth: [`cmake/PixelStudioVersion.cmake`](../../cmake/PixelStudioVersion.cmake).  
Git tags: `v0.1.0`, `v1.2.3b`.

## PR labels (Release Drafter)

| Label | Effect |
|-------|--------|
| `patch` | Default: semver patch bump → bump `PS_VERSION_MINOR`, clear `FIX` |
| `minor` | Semver minor → bump `PS_VERSION_MAJOR`, reset `MINOR`, clear `FIX` |
| `major` | Semver major → bump `PS_VERSION_GLOBAL`, reset others, clear `FIX` |
| `version-fix` | Suffix only: set `PS_VERSION_FIX` in cmake; tag manually e.g. `v1.2.3b` |

Release Drafter does **not** append fix letters automatically. For fix-only releases, update cmake and tag `v1.2.3b`, then push.

## Release

1. Align `cmake/PixelStudioVersion.cmake`
2. `git tag v1.2.3b && git push origin v1.2.3b`
3. [Release workflow](../../.github/workflows/release.yml) publishes the installer
