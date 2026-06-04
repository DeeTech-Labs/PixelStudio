# Сборка PixelStudio

> **English:** [en/building.md](en/building.md)

## Требования

1. **Visual Studio 2022** — рабочая нагрузка «Разработка классических приложений на C++», MSVC v143 x64.
2. **Qt 6.8.2** (или совместимая 6.8.x) для `msvc2022_64`:
   - Qt Quick
   - Qt Quick Controls 2
   - Qt Svg
   - Qt Linguist (инструменты)
3. **CMake** 3.16+ и **Ninja** в `PATH`.
4. По желанию: **Inno Setup 6** для установщика Windows.

## Конфигурация и компиляция

Из корня репозитория в PowerShell:

```powershell
$qt = "C:\Qt6\6.8.2\msvc2022_64"

cmake -S . -B build\Release -G Ninja `
  -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_PREFIX_PATH="$qt"

cmake --build build\Release
```

Исполняемый файл: `build\Release\appPixelStudio.exe`.

Если CMake не находит Qt, проверьте, что `CMAKE_PREFIX_PATH` указывает на корень kit (каталог с `bin\qmake.exe`).

## Запуск без установки

После сборки DLL и QML-плагины не копируются автоматически. В Qt Creator / Visual Studio используйте окружение kit.

Запуск из PowerShell:

```powershell
$qt = "C:\Qt6\6.8.2\msvc2022_64"
$env:PATH = "$qt\bin;$env:PATH"

& ".\build\Release\appPixelStudio.exe"
```

## Развёртывание runtime (для распространения)

```powershell
$qt = "C:\Qt6\6.8.2\msvc2022_64"
$exe = ".\build\Release\appPixelStudio.exe"
$qml = ".\src\qml"

& "$qt\bin\windeployqt.exe" --release --compiler-runtime --qmldir $qml $exe
```

Команда заполнит `build\Release` нужными DLL и QML-зависимостями.

## Полный пайплайн установщика

Скрипт при необходимости конфигурирует и собирает проект, запускает `windeployqt`, копирует фиксированный список файлов и вызывает Inno Setup:

```powershell
.\installer\build-installer.ps1 -QtDir "C:\Qt6\6.8.2\msvc2022_64"
```

| Параметр | По умолчанию | Описание |
|----------|--------------|----------|
| `-BuildDir` | `build\Release` | Каталог сборки CMake |
| `-QtDir` | `C:\Qt6\6.8.2\msvc2022_64` | Корень Qt kit |
| `-IsccPath` | авто | Путь к `ISCC.exe` |
| `-SkipBuild` | — | Только deploy / stage / installer |
| `-SkipDeploy` | — | Пропустить `windeployqt` |

Результат: `installer\output\PixelStudio-Setup-<версия>.exe`.

Версия: [`cmake/PixelStudioVersion.cmake`](../cmake/PixelStudioVersion.cmake).

## CI

| Workflow | Триггер | Результат |
|----------|---------|-----------|
| [`.github/workflows/build.yml`](../.github/workflows/build.yml) | push/PR → `main` | `appPixelStudio.exe` (artifact) |
| [`.github/workflows/release.yml`](../.github/workflows/release.yml) | tag `v*.*.*` или вручную | Установщик + GitHub Release |
| [`.github/workflows/labels.yml`](../.github/workflows/labels.yml) | изменение `.github/labels.yml` | Синхронизация меток |

Окружение: [`.github/actions/setup-windows-qt`](../.github/actions/setup-windows-qt) (Qt 6.8.2, модуль **qt5compat** для `Qt5Compat.GraphicalEffects`, Linguist tools; release — Ninja + `build-installer.ps1`).

## Устранение неполадок

| Проблема | Что сделать |
|----------|-------------|
| `Could not find Qt6` | Указать `-DCMAKE_PREFIX_PATH` на корень MSVC kit |
| Ninja / компилятор не найден | «x64 Native Tools» или Developer PowerShell |
| `windeployqt` не подхватывает QML | `--qmldir` → `src\qml` |
| Ошибка staging (`Missing … qml/Qt5Compat/…`) | В CI/local Qt должен быть модуль **qt5compat**; пересобрать с `windeployqt` |
| Ошибка staging (`imageformats/qicns.dll` и др.) | В списке только нужные плагины (jpeg/gif/ico/svg); лишние форматы не требуются |
| SmartScreen при запуске установщика | Неподписанный `.exe` — нормально; для продакшена нужна подпись Authenticode (сертификат) |
| Приложение не стартует после установки | Запуск из `Program Files\PixelStudio`, проверить наличие `qml\Qt5Compat\GraphicalEffects` рядом с exe |
