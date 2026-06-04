# PixelStudio

[![CI](https://github.com/DeeTech-Labs/PixelStudio/actions/workflows/build.yml/badge.svg?branch=main)](https://github.com/DeeTech-Labs/PixelStudio/actions/workflows/build.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](../../LICENSE)
[![Qt](https://img.shields.io/badge/Qt-6.8-41cd52?logo=qt)](https://www.qt.io/)
[![Platform](https://img.shields.io/badge/platform-Windows-0078d4?logo=windows)](https://github.com/DeeTech-Labs/PixelStudio/releases)

> **Русский (default):** [../../README.md](../../README.md) · [Documentation index](../README.md)

## Languages / Языки

| Language | Document |
|----------|----------|
| **English** | this file |
| **Русский** | [../../README.md](../../README.md) |

Full index: [../README.md](../README.md)

---

Desktop utility to convert images into C/C++ arrays and firmware-friendly formats for OLED/TFT and embedded displays.

## Screenshots

Add PNG files under [`../screenshots/`](../screenshots/) ([guide](../screenshots/README.md)), then uncomment in the README:

<!--
![Welcome screen](../screenshots/welcome.png)
![Studio workspace](../screenshots/workspace.png)
-->

## Features

- **Image → firmware code** — rasterization, encoding analysis, C/C++ header generation
- **Display profiles** — OLED/TFT workflows (mono, RGB565, indexed palette)
- **Presets** — Icon, Splash, E-paper, Indexed palette
- **Studio UI** — drag-and-drop, live preview, inspector (image / display / export)
- **Export** — batch export, sprite atlas, binary export, watch folder
- **Projects** — save/load studio sessions
- **i18n** — English and Russian UI

## Download

Windows installer: **[Releases](https://github.com/DeeTech-Labs/PixelStudio/releases)** (`PixelStudio-Setup-*.exe`).

Verify SHA256 checksums on the release page when provided.

## Requirements (build from source)

| Component | Version |
|-----------|---------|
| OS | Windows 10/11 x64 |
| [Qt](https://www.qt.io/download) | 6.8.x (MSVC 2022 64-bit): Quick, QuickControls2, Gui, Network, Concurrent, LinguistTools, Svg |
| [CMake](https://cmake.org/) | ≥ 3.16 |
| [Ninja](https://ninja-build.org/) | recommended |
| [Visual Studio](https://visualstudio.microsoft.com/) | 2022 with C++ desktop workload |
| [Inno Setup](https://jrsoftware.org/isinfo.php) | 6.x (installer only) |

Details: [building.md](building.md) · [Русский](../building.md)

## Quick build

```powershell
$qt = "C:\Qt6\6.8.2\msvc2022_64"

cmake -S . -B build\Release -G Ninja `
  -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_PREFIX_PATH="$qt"

cmake --build build\Release
```

Run `build\Release\appPixelStudio.exe` — use `windeployqt` before redistribution ([building guide](building.md)).

## Installer

```powershell
.\installer\build-installer.ps1 -QtDir "C:\Qt6\6.8.2\msvc2022_64"
# Output: installer\output\PixelStudio-Setup-<version>.exe
```

Version: [`cmake/PixelStudioVersion.cmake`](../../cmake/PixelStudioVersion.cmake).

## Repository layout

```
cmake/          Version and CMake helpers
src/            C++ core, QML UI, persistence, export
i18n/           Translation sources (.ts)
resources/      Icons and assets
installer/      Inno Setup and packaging
docs/           Documentation (RU + docs/en/)
```

## Contributing

[CONTRIBUTING.md](CONTRIBUTING.md) · [Русский](../../CONTRIBUTING.md)

Issues: [GitHub Issues](https://github.com/DeeTech-Labs/PixelStudio/issues) (form templates in Russian).

## Security

[SECURITY.md](../../SECURITY.md) (Russian)

## License

[MIT](../../LICENSE) — third-party notices: [NOTICE.txt](../../NOTICE.txt)
