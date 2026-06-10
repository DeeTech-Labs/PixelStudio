# Сборка PixelStudio

## Windows

### Требования

1. **Visual Studio 2022** — рабочая нагрузка «Разработка классических приложений на C++», MSVC v143 x64.
2. **Qt 6.8.2** (или совместимая 6.8.x) для `msvc2022_64`:
   - Qt Quick
   - Qt Quick Controls 2
   - Qt Svg
3. **CMake** 3.16+ и **Ninja** в `PATH`.
4. По желанию: **Inno Setup 6** для установщика Windows.

### Конфигурация и компиляция

Из корня репозитория в PowerShell:

```powershell
$qt = "C:\Qt6\6.8.2\msvc2022_64"

cmake -S . -B build\Release -G Ninja `
  -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_PREFIX_PATH="$qt"

cmake --build build\Release
```

Исполняемый файл: `build\Release\PixelStudio.exe`.

Если CMake не находит Qt, проверьте, что `CMAKE_PREFIX_PATH` указывает на корень kit (каталог с `bin\qmake.exe`).

### Запуск без установки

После сборки DLL и QML-плагины не копируются автоматически. В Qt Creator / Visual Studio используйте окружение kit.

Запуск из PowerShell:

```powershell
$qt = "C:\Qt6\6.8.2\msvc2022_64"
$env:PATH = "$qt\bin;$env:PATH"

& ".\build\Release\PixelStudio.exe"
```

### Развёртывание runtime (для распространения)

```powershell
$qt = "C:\Qt6\6.8.2\msvc2022_64"
$exe = ".\build\Release\PixelStudio.exe"
$qml = ".\src\qml"

& "$qt\bin\windeployqt.exe" --release --compiler-runtime --qmldir $qml $exe
```

Команда заполнит `build\Release` нужными DLL и QML-зависимостями.

### Полный пайплайн установщика

Скрипт при необходимости конфигурирует и собирает проект, запускает `windeployqt`, формирует staging (без артефактов сборки и лишних runtime-файлов), затем вызывает Inno Setup:

```powershell
.\installer\windows-build-installer.ps1 -QtDir "C:\Qt6\6.8.2\msvc2022_64"
```

| Параметр | По умолчанию | Описание |
|----------|--------------|----------|
| `-BuildDir` | `build\Release` | Каталог сборки CMake |
| `-QtDir` | `C:\Qt6\6.8.2\msvc2022_64` | Корень Qt kit |
| `-IsccPath` | авто | Путь к `ISCC.exe` |
| `-SkipBuild` | — | Только deploy / stage / installer |
| `-SkipDeploy` | — | Пропустить `windeployqt` |
| `-SkipPrune` | — | Не удалять лишние runtime-файлы (только артефакты сборки) |

Результат: `installer\output\PixelStudio-Setup-<версия>.exe`.

Версия: [`cmake/PixelStudioVersion.cmake`](../cmake/PixelStudioVersion.cmake).

## macOS

### Требования

1. **macOS 12+** (Apple Silicon или Intel; релиз — **universal binary** arm64 + x86_64).
2. **Xcode Command Line Tools** (`xcode-select --install`).
3. **Qt 6.8.2** (или совместимая 6.8.x) для `clang_64`:
   - Qt Quick, Qt Quick Controls 2, Qt Svg
   - модули **qt5compat** и **qtshadertools** (для QML deploy)
4. **CMake** 3.16+ и **Ninja** (`brew install cmake ninja`).

### Конфигурация и компиляция

Из корня репозитория в Terminal:

```bash
export QT_DIR="$HOME/Qt/6.8.2/macos"

chmod +x installer/macos-make-icns.sh
installer/macos-make-icns.sh

cmake -S . -B build/Release -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH="$QT_DIR" \
  -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"

cmake --build build/Release
installer/macos-verify-universal.sh build/Release/PixelStudio.app
```

Результат: `build/Release/PixelStudio.app` (universal: arm64 + x86_64).

Только arm64 (локальная отладка на Apple Silicon): `-DCMAKE_OSX_ARCHITECTURES=arm64`.

### Запуск без deploy

```bash
open build/Release/PixelStudio.app
```

Для запуска из терминала с Qt в `PATH`:

```bash
export PATH="$QT_DIR/bin:$PATH"
build/Release/PixelStudio.app/Contents/MacOS/PixelStudio
```

### Развёртывание runtime

```bash
export QT_DIR="$HOME/Qt/6.8.2/macos"
"$QT_DIR/bin/macdeployqt" build/Release/PixelStudio.app \
  -qmldir=src/qml -always-overwrite -no-strip \
  -no-translations -skip-plugin-types=qmltooling
```

### Полный пайплайн DMG

```bash
export QT_DIR="$HOME/Qt/6.8.2/macos"
chmod +x installer/macos-*.sh cmake/read-pixel-studio-version.sh
installer/macos-build-bundle.sh
installer/macos-make-dmg.sh
```

Результат: `installer/output/PixelStudio_<версия>_macos_universal.dmg` (unsigned).

Переменные окружения для скриптов: `QT_DIR` или `QT_ROOT_DIR`, опционально `BUILD_DIR`, `SKIP_BUILD`, `SKIP_DEPLOY`, `SKIP_PRUNE`.

### Gatekeeper (unsigned)

Релизные DMG не подписаны. При первом запуске macOS может заблокировать приложение. Для локального теста:

```bash
xattr -cr /Applications/PixelStudio.app
```

## CI

| Workflow | Триггер | Результат |
|----------|---------|-----------|
| [`.github/workflows/build.yml`](../.github/workflows/build.yml) | push/PR → `main` | `PixelStudio.exe` + `PixelStudio.app` (artifacts) |
| [`.github/workflows/release.yml`](../.github/workflows/release.yml) | tag `v*.*.*` или вручную | Windows installer + macOS DMG → GitHub Release |
| [`.github/workflows/labels.yml`](../.github/workflows/labels.yml) | изменение `.github/labels.yml` | Синхронизация меток |

Окружение Windows: [`.github/actions/setup-windows-qt`](../.github/actions/setup-windows-qt). macOS: [`.github/actions/setup-macos-qt`](../.github/actions/setup-macos-qt). Модули **qt5compat** и **qtshadertools** нужны на обеих платформах.

## Устранение неполадок

| Проблема | Что сделать |
|----------|-------------|
| `Could not find Qt6` | Указать `-DCMAKE_PREFIX_PATH` на корень MSVC kit |
| Ninja / компилятор не найден | «x64 Native Tools» или Developer PowerShell |
| `windeployqt` не подхватывает QML | `--qmldir` → `src\qml` |
| `windeployqt` / `Qt6ShaderTools.dll` not found | В CI установить модуль **qtshadertools** (см. setup-windows-qt) |
| `windeployqt` / `qml/Qt5Compat/…` not found | В CI/local Qt должен быть модуль **qt5compat** |
| Приложение не стартует после установки | Проверить `installer\staging-windows\PixelStudio.exe`; при необходимости `-SkipPrune` |
| SmartScreen при запуске установщика | Неподписанный `.exe` — нормально; для продакшена нужна подпись Authenticode (сертификат) |
| Приложение не стартует после установки | Запуск из `Program Files\PixelStudio`, проверить наличие `qml\Qt5Compat\GraphicalEffects` рядом с exe |
| macOS: «повреждено» / Gatekeeper | Unsigned build — `xattr -cr PixelStudio.app` или Системные настройки → Конфиденциальность |
| macOS: `PixelStudio.icns` missing | Запустить `installer/macos-make-icns.sh` (требует macOS) |
| macOS: `macdeployqt` / Qt5Compat | Установить модули **qt5compat** и **qtshadertools** в Qt kit |
| macOS: universal / `lipo` failed | Qt kit `clang_64` должен быть universal; пересоберите с `-DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"` |
