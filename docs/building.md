# Сборка PixelStudio

[← README](../README.md) · [Участие](../CONTRIBUTING.md)

> **Платформа:** Windows x64 &nbsp;·&nbsp; **Qt:** 6.8.x (MSVC 2022) &nbsp;·&nbsp; **Сборщик:** CMake + Ninja

## Содержание

- [Требования](#требования)
- [1. Конфигурация и компиляция](#1-конфигурация-и-компиляция)
- [2. Запуск без установки](#2-запуск-без-установки)
- [3. Развёртывание runtime](#3-развёртывание-runtime)
- [4. Установщик Windows](#4-установщик-windows)
- [CI](#ci)
- [Устранение неполадок](#устранение-неполадок)

## Требования

| Компонент | Версия / примечание |
|-----------|---------------------|
| Visual Studio 2022 | Рабочая нагрузка «Разработка классических приложений на C++», MSVC v143 x64 |
| Qt | 6.8.2 или совместимая 6.8.x, kit `msvc2022_64` |
| Модули Qt | Quick, Quick Controls 2, Svg |
| CMake | 3.16+ |
| Ninja | В `PATH` |
| Inno Setup 6 | Опционально — для сборки установщика |

Во всех примерах ниже `$qt` — корень Qt kit (каталог с `bin\qmake.exe`), например `C:\Qt6\6.8.2\msvc2022_64`.

## 1. Конфигурация и компиляция

Из корня репозитория в PowerShell:

```powershell
$qt = "C:\Qt6\6.8.2\msvc2022_64"

cmake -S . -B build\Release -G Ninja `
  -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_PREFIX_PATH="$qt"

cmake --build build\Release
```

**Результат:** `build\Release\PixelStudio.exe`

Если CMake не находит Qt — проверьте `CMAKE_PREFIX_PATH`.

## 2. Запуск без установки

После сборки DLL и QML-плагины не копируются автоматически. В Qt Creator / Visual Studio используйте окружение kit.

```powershell
$qt = "C:\Qt6\6.8.2\msvc2022_64"
$env:PATH = "$qt\bin;$env:PATH"

& ".\build\Release\PixelStudio.exe"
```

## 3. Развёртывание runtime

Для распространения собранного `.exe` без установщика:

```powershell
$qt = "C:\Qt6\6.8.2\msvc2022_64"
$exe = ".\build\Release\PixelStudio.exe"
$qml = ".\src\qml"

& "$qt\bin\windeployqt.exe" --release --compiler-runtime --qmldir $qml $exe
```

Команда заполнит `build\Release` нужными DLL и QML-зависимостями.

## 4. Установщик Windows

Скрипт конфигурирует и собирает проект, запускает `windeployqt`, формирует staging и вызывает Inno Setup:

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
| `-SkipPrune` | — | Не удалять лишние runtime-файлы |

**Результат:** `installer\output\PixelStudio-Setup-<версия>.exe`

Версия задаётся в [`cmake/PixelStudioVersion.cmake`](../cmake/PixelStudioVersion.cmake).

## CI

Push и pull request в `main` проверяются workflow [build.yml](../.github/workflows/build.yml): Release-сборка и артефакт `PixelStudio.exe`.

Окружение: [setup-windows-qt](../.github/actions/setup-windows-qt) — Qt 6.8.2, модули **qt5compat** и **qtshadertools**.

## Устранение неполадок

| Проблема | Решение |
|----------|---------|
| `Could not find Qt6` | `-DCMAKE_PREFIX_PATH` → корень MSVC kit |
| Ninja / компилятор не найден | Запуск из «x64 Native Tools» или Developer PowerShell |
| `windeployqt` не подхватывает QML | `--qmldir` → `src\qml` |
| `Qt6ShaderTools.dll` not found | Установить модуль **qtshadertools** |
| `qml/Qt5Compat/…` not found | Установить модуль **qt5compat** |
| Не стартует после установки | Проверить `installer\staging\` или `Program Files\PixelStudio\`; рядом с exe — `qml\Qt5Compat\GraphicalEffects`; попробовать `-SkipPrune` |
| SmartScreen на установщике | Неподписанный `.exe` — ожидаемо; для продакшена — Authenticode |

---

[← README](../README.md) · [Участие](../CONTRIBUTING.md)
