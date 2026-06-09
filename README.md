<div align="center">

<img src="resources/ico/PixelStudio.png" alt="PixelStudio" width="96" height="96">

# PixelStudio

**Изображение → прошивочный код за минуты**

Десктопная студия для конвертации картинок в C/C++ массивы и бинарные форматы под OLED, TFT и другие embedded-дисплеи.

[![CI](https://github.com/DeeTech-Labs/PixelStudio/actions/workflows/build.yml/badge.svg?branch=main)](https://github.com/DeeTech-Labs/PixelStudio/actions/workflows/build.yml)
[![Release](https://img.shields.io/github/v/release/DeeTech-Labs/PixelStudio?label=release&sort=semver)](https://github.com/DeeTech-Labs/PixelStudio/releases)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Qt 6.8](https://img.shields.io/badge/Qt-6.8-41cd52?logo=qt)](https://www.qt.io/)
[![Platform](https://img.shields.io/badge/platform-Windows%20x64-0078d4?logo=windows)](https://github.com/DeeTech-Labs/PixelStudio/releases)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-00599C?logo=cplusplus)](https://isocpp.org/)

[**Скачать установщик**](https://github.com/DeeTech-Labs/PixelStudio/releases) ·
[**Сборка**](docs/building.md) ·
[**Issues**](https://github.com/DeeTech-Labs/PixelStudio/issues) ·
[**Contributing**](CONTRIBUTING.md)

</div>

---

## Как это работает

```mermaid
flowchart LR
    A["🖼️ Исходник<br/>PNG · JPG · SVG · BMP"]
    B["⚙️ PixelStudio<br/>профиль · дither · палитра"]
    C["📐 Растер<br/>под разрешение дисплея"]
    D["📄 Выход<br/>.h · .bin · atlas"]
    E["🔌 Прошивка<br/>MCU · ESP · STM32"]

    A --> B --> C --> D --> E

    style A fill:#1e293b,stroke:#64748b,color:#e2e8f0
    style B fill:#0f766e,stroke:#14b8a6,color:#ecfdf5
    style C fill:#1d4ed8,stroke:#3b82f6,color:#eff6ff
    style D fill:#7c3aed,stroke:#a78bfa,color:#f5f3ff
    style E fill:#b45309,stroke:#f59e0b,color:#fffbeb
```

<table>
<tr>
<td width="50%" valign="top">

### Вход
- Drag-and-drop и watch folder
- Импорт существующих `.h`
- Пакетная обработка папок

</td>
<td width="50%" valign="top">

### Выход
- C/C++ заголовки с массивами
- Бинарные дампы для flash
- Sprite atlas и batch export

</td>
</tr>
</table>

---

## Возможности

```mermaid
mindmap
  root((PixelStudio))
    Студия
      Вкладки проектов
      Live preview
      Инспектор
    Дисплеи
      Mono 1-bpp
      RGB565
      Indexed palette
      E-paper пресеты
    Экспорт
      Batch
      Sprite atlas
      Watch folder
      Binary dump
    Локализация
      Русский
      English
      JSON overrides
```

<table>
<tr>
<td align="center" width="25%">
<h3>🎨</h3>
<b>Растеризация</b><br/>
<sub>Масштаб, кроп, фильтры, dithering под конкретный экран</sub>
</td>
<td align="center" width="25%">
<h3>📟</h3>
<b>Профили дисплея</b><br/>
<sub>OLED/TFT: mono, RGB565, indexed — пресеты Icon, Splash, E-paper</sub>
</td>
<td align="center" width="25%">
<h3>🧩</h3>
<b>Генерация кода</b><br/>
<sub>Анализ кодирования, C/C++ headers, PROGMEM-friendly layout</sub>
</td>
<td align="center" width="25%">
<h3>📦</h3>
<b>Экспорт</b><br/>
<sub>Batch, atlas, binary, watch folder — без ручной рутины</sub>
</td>
</tr>
<tr>
<td align="center">
<h3>💾</h3>
<b>Проекты</b><br/>
<sub>Сохранение сессий студии и настроек экспорта</sub>
</td>
<td align="center">
<h3>🔍</h3>
<b>Инспектор</b><br/>
<sub>Image · Display · Export — всё в одной панели</sub>
</td>
<td align="center">
<h3>🌐</h3>
<b>Переводы</b><br/>
<sub>RU/EN из коробки; свои языки через JSON в AppData</sub>
</td>
<td align="center">
<h3>🪟</h3>
<b>Windows installer</b><br/>
<sub>Inno Setup, CI-сборка, SHA256 в описании релиза</sub>
</td>
</tr>
</table>

### Поддерживаемые форматы пикселей

| Формат | Типичное применение | Пример дисплея |
|--------|---------------------|----------------|
| **1-bpp mono** | Иконки, статус-бары | SSD1306 OLED |
| **RGB565** | Цветной UI, сплэши | ILI9341 TFT |
| **Indexed 4/8-bit** | Спрайты, tilemaps | Game Boy-style |
| **Custom palette** | Брендовые цвета | Любой fixed-palette |

---

## Скриншоты

<table>
<tr>
<td width="50%"><img src="docs/screenshots/welcome.png" alt="Экран приветствия"/></td>
<td width="50%"><img src="docs/screenshots/workspace.png" alt="Рабочая область студии"/></td>
</tr>
<tr>
<td align="center"><sub>Welcome — старт и недавние проекты</sub></td>
<td align="center"><sub>Studio — превью, инспектор, экспорт</sub></td>
</tr>
</table>

---

## Скачать

| | |
|---|---|
| **Windows x64** | [`PixelStudio-Setup-*.exe`](https://github.com/DeeTech-Labs/PixelStudio/releases) |
| **Проверка** | SHA256-сумма в описании [релиза](https://github.com/DeeTech-Labs/PixelStudio/releases) |
| **Исходники** | Этот репозиторий + [инструкция по сборке](docs/building.md) |

---

## Быстрый старт

### Установка (пользователь)

1. Скачайте `PixelStudio-Setup-*.exe` из [Releases](https://github.com/DeeTech-Labs/PixelStudio/releases).
2. Запустите установщик → откройте PixelStudio.
3. Перетащите изображение → выберите профиль дисплея → экспортируйте `.h`.

### Сборка (разработчик)

<details>
<summary><b>CMake + Ninja</b> — развернуть</summary>

```powershell
$qt = "C:\Qt6\6.8.2\msvc2022_64"

cmake -S . -B build\Release -G Ninja `
  -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_PREFIX_PATH="$qt"

cmake --build build\Release
```

Запуск: `build\Release\PixelStudio.exe`  
Перед распространением: `windeployqt` — см. [docs/building.md](docs/building.md).

</details>

<details>
<summary><b>Установщик Inno Setup</b> — развернуть</summary>

```powershell
.\installer\build-installer.ps1 -QtDir "C:\Qt6\6.8.2\msvc2022_64"
# → installer\output\PixelStudio-Setup-<версия>.exe
```

Версия: [`cmake/PixelStudioVersion.cmake`](cmake/PixelStudioVersion.cmake).

</details>

---

## Архитектура

```mermaid
graph TB
    subgraph UI["QML · Qt Quick"]
        W[Welcome]
        S[Studio + вкладки]
        I[Inspector]
        P[Preferences]
    end

    subgraph Core["C++ · pixelstudio_core"]
        PR[processing]
        EX[export]
        PE[persistence]
        TR[translation]
    end

    W --> S
    S --> I
    S --> PR
    S --> EX
    S --> PE
    P --> TR
    PR --> EX

    style UI fill:#0f172a,stroke:#334155,color:#e2e8f0
    style Core fill:#134e4a,stroke:#2dd4bf,color:#ecfdf5
```

```
PixelStudio/
├── cmake/           Версия и CMake-модули
├── src/             C++ ядро + QML UI
│   ├── processing/  Растер, палитры, codegen
│   ├── export/      Batch, atlas, watch folder
│   └── persistence/ Проекты и настройки
├── translations/    en.json · ru.json (+ AppData overrides)
├── resources/       Иконки и ассеты
├── installer/       Inno Setup + CI pipeline
└── docs/            Сборка, версионирование
```

---

## Стек

| Слой | Технологии |
|------|------------|
| UI | Qt 6.8 Quick, Quick Controls 2, QML |
| Core | C++17, CMake, Ninja / MSVC 2022 |
| i18n | JSON bundles, runtime overrides |
| Packaging | windeployqt, Inno Setup 6 |
| CI/CD | GitHub Actions — `build.yml`, `release.yml` |

---

## Участие

Нашли баг или есть идея? → [**Issues**](https://github.com/DeeTech-Labs/PixelStudio/issues)  
Хотите внести код? → [**CONTRIBUTING.md**](CONTRIBUTING.md)  
Уязвимость? → [**SECURITY.md**](SECURITY.md) · private advisory

---

<div align="center">

**[MIT](LICENSE)** · [NOTICE.txt](NOTICE.txt) (Qt и сторонние компоненты)

</div>
