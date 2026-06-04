# Building PixelStudio

> **Русский:** [../building.md](../building.md)

## Prerequisites

1. **Visual Studio 2022** with *Desktop development with C++* and MSVC v143 x64.
2. **Qt 6.8.2** (or compatible 6.8.x) for `msvc2022_64`, including Qt Quick, Quick Controls 2, Svg, and Linguist tools.
3. **CMake** 3.16+ and **Ninja** on `PATH`.
4. Optional: **Inno Setup 6** for the Windows installer.

## Configure and compile

From the repository root in PowerShell:

```powershell
$qt = "C:\Qt6\6.8.2\msvc2022_64"

cmake -S . -B build\Release -G Ninja `
  -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_PREFIX_PATH="$qt"

cmake --build build\Release
```

Output: `build\Release\appPixelStudio.exe`.

If configure fails to find Qt, set `CMAKE_PREFIX_PATH` to the kit root (folder containing `bin\qmake.exe`).

## Run without installing

Qt DLLs and QML plugins are not copied automatically after build. Use the Qt kit environment in Qt Creator / Visual Studio, or:

```powershell
$qt = "C:\Qt6\6.8.2\msvc2022_64"
$env:PATH = "$qt\bin;$env:PATH"
& ".\build\Release\appPixelStudio.exe"
```

## Deploy runtime (redistribution)

```powershell
$qt = "C:\Qt6\6.8.2\msvc2022_64"
& "$qt\bin\windeployqt.exe" --release --compiler-runtime --qmldir ".\src\qml" ".\build\Release\appPixelStudio.exe"
```

## Full installer pipeline

```powershell
.\installer\build-installer.ps1 -QtDir "C:\Qt6\6.8.2\msvc2022_64"
```

| Parameter | Default | Description |
|-----------|---------|-------------|
| `-BuildDir` | `build\Release` | CMake build directory |
| `-QtDir` | `C:\Qt6\6.8.2\msvc2022_64` | Qt kit root |
| `-IsccPath` | auto | Path to `ISCC.exe` |
| `-SkipBuild` | — | Deploy/stage/installer only |
| `-SkipDeploy` | — | Skip `windeployqt` |

Output: `installer\output\PixelStudio-Setup-<version>.exe`. Version: [`cmake/PixelStudioVersion.cmake`](../../cmake/PixelStudioVersion.cmake).

## CI

| Workflow | Trigger | Result |
|----------|---------|--------|
| [`build.yml`](../../.github/workflows/build.yml) | push/PR → `main` | `appPixelStudio.exe` artifact |
| [`release.yml`](../../.github/workflows/release.yml) | tag `v*.*.*` or manual | Installer + GitHub Release |
| [`labels.yml`](../../.github/workflows/labels.yml) | `.github/labels.yml` changed | Label sync |

Uses [`.github/actions/setup-windows-qt`](../../.github/actions/setup-windows-qt).

## Troubleshooting

| Problem | Suggestion |
|---------|------------|
| `Could not find Qt6` | Set `-DCMAKE_PREFIX_PATH` to MSVC kit root |
| Ninja / compiler missing | x64 Native Tools or VS Developer PowerShell |
| `windeployqt` missing QML | `--qmldir` → `src\qml` |
| Installer staging mismatch | Rebuild with deploy; see `installer/build-installer.ps1` |
