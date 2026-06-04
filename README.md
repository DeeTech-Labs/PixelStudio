# PixelStudio

[![CI](https://github.com/DeeTech-Labs/PixelStudio/actions/workflows/build.yml/badge.svg?branch=main)](https://github.com/DeeTech-Labs/PixelStudio/actions/workflows/build.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Qt](https://img.shields.io/badge/Qt-6.8-41cd52?logo=qt)](https://www.qt.io/)
[![Platform](https://img.shields.io/badge/platform-Windows-0078d4?logo=windows)](https://github.com/DeeTech-Labs/PixelStudio/releases)

## Языки / Languages

| Язык | Документация |
|------|----------------|
| **Русский** (по умолчанию) | этот файл |
| **English** | [docs/en/README.md](docs/en/README.md) |

Полный указатель: [docs/README.md](docs/README.md) · [docs/en/README.md](docs/en/README.md) (index)

---

Десктопная утилита для конвертации изображений в C/C++ массивы и прошивочные форматы для OLED/TFT и embedded-дисплеев.

## Скриншоты

Добавьте PNG в [`docs/screenshots/`](docs/screenshots/) ([инструкция](docs/screenshots/README.md)), затем раскомментируйте:

<!--
![Экран приветствия](docs/screenshots/welcome.png)
![Рабочая область](docs/screenshots/workspace.png)
-->

## Возможности

- **Изображение → код прошивки** — растеризация, анализ кодирования, генерация C/C++ заголовков
- **Профили дисплея** — сценарии для OLED/TFT (моно, RGB565, индексированная палитра)
- **Пресеты** — Icon, Splash, E-paper, Indexed palette
- **Студия** — drag-and-drop, превью, инспектор (изображение / дисплей / экспорт)
- **Экспорт** — пакетный экспорт, sprite atlas, бинарный экспорт, watch folder
- **Проекты** — сохранение и загрузка сессий
- **Интерфейс** — русский и английский UI

## Скачать

Готовый установщик для Windows: **[Releases](https://github.com/DeeTech-Labs/PixelStudio/releases)** (`PixelStudio-Setup-*.exe`).

На странице релиза указаны SHA256-суммы (если опубликованы).

## Требования (сборка из исходников)

| Компонент | Версия |
|-----------|--------|
| ОС | Windows 10/11 x64 |
| [Qt](https://www.qt.io/download) | 6.8.x (MSVC 2022 64-bit): Quick, QuickControls2, Gui, Network, Concurrent, LinguistTools, Svg |
| [CMake](https://cmake.org/) | ≥ 3.16 |
| [Ninja](https://ninja-build.org/) | рекомендуется |
| [Visual Studio](https://visualstudio.microsoft.com/) | 2022, рабочая нагрузка «Разработка классических приложений на C++» |
| [Inno Setup](https://jrsoftware.org/isinfo.php) | 6.x (только для установщика) |

Подробнее: [docs/building.md](docs/building.md) · [English](docs/en/building.md)

## Быстрая сборка

```powershell
$qt = "C:\Qt6\6.8.2\msvc2022_64"   # путь к вашему Qt

cmake -S . -B build\Release -G Ninja `
  -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_PREFIX_PATH="$qt"

cmake --build build\Release
```

Запуск: `build\Release\appPixelStudio.exe` (перед распространением — `windeployqt`, см. [сборку](docs/building.md)).

## Установщик

```powershell
.\installer\build-installer.ps1 -QtDir "C:\Qt6\6.8.2\msvc2022_64"
# Результат: installer\output\PixelStudio-Setup-<версия>.exe
```

Версия задаётся в [`cmake/PixelStudioVersion.cmake`](cmake/PixelStudioVersion.cmake).

## Структура репозитория

```
cmake/          Версия и CMake
src/            C++ ядро, QML, persistence, export
i18n/           Переводы (.ts)
resources/      Иконки и ресурсы
installer/      Inno Setup и упаковка
docs/           Документация (RU + docs/en/)
```

## Участие в проекте

[CONTRIBUTING.md](CONTRIBUTING.md) · [English](docs/en/CONTRIBUTING.md)

Сообщения об ошибках и предложения: [Issues](https://github.com/DeeTech-Labs/PixelStudio/issues) (шаблоны на русском).

## Безопасность

[SECURITY.md](SECURITY.md) · [English](docs/en/SECURITY.md)

## Лицензия

[MIT](LICENSE) — сторонние компоненты (Qt): [NOTICE.txt](NOTICE.txt)
