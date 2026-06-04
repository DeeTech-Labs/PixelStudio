# Contributing to PixelStudio

> **Русский:** [../../CONTRIBUTING.md](../../CONTRIBUTING.md)

Thank you for improving PixelStudio. This document covers build, verification, and submission basics.

## Before you start

- Search [existing issues](https://github.com/DeeTech-Labs/PixelStudio/issues).
- Discuss large changes in an issue first.
- Keep pull requests focused.

## Environment

| Tool | Notes |
|------|--------|
| Windows 10/11 x64 | Primary platform |
| Qt 6.8.x MSVC 2022 64-bit | Quick, QuickControls2, Gui, Network, Concurrent, LinguistTools, Svg |
| CMake ≥ 3.16 | Ninja recommended |
| Visual Studio 2022 | C++ desktop workload |
| Inno Setup 6 | Installer only |

Build guide: [building.md](building.md) · [Русский](../building.md)

## Build

```powershell
$qt = "C:\Qt6\6.8.2\msvc2022_64"

cmake -S . -B build\Release -G Ninja `
  -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_PREFIX_PATH="$qt"

cmake --build build\Release
```

## Versions

Edit [`cmake/PixelStudioVersion.cmake`](../../cmake/PixelStudioVersion.cmake) and [`CHANGELOG.md`](../../CHANGELOG.md) under `[Unreleased]`.

## Code style

- Follow [.editorconfig](../../.editorconfig).
- Respect module boundaries; prefer `pixelstudio_core` for new logic.

## Translations

- QML context: `PixelStudio`.
- Update `i18n/pixelstudio_en.ts` and `i18n/pixelstudio_ru.ts` for UI changes.

## Pull requests

1. Fork, branch from `main`.
2. Local Windows build passes; CI `build.yml` green.
3. Update [CHANGELOG.md](../../CHANGELOG.md) when user-visible.
4. Complete the PR template.
5. Screenshots in `docs/screenshots/` for UI changes.

## Releases (maintainers)

| Event | Workflow |
|-------|----------|
| PR / push to `main` | `build.yml` |
| Tag `v0.1.0` | `release.yml` |
| Manual | Actions → Release → Run workflow |

Labels: [`.github/labels.yml`](../../.github/labels.yml). Issue forms are in Russian (primary audience).

## Code of conduct

[CODE_OF_CONDUCT](../../CODE_OF_CONDUCT.md) · [Contributor Covenant (EN)](https://www.contributor-covenant.org/version/2/1/code_of_conduct.html)
