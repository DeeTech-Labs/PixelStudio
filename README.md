<div align="center">

<img src="resources/ico/PixelStudio.png" alt="PixelStudio" width="120" height="120">

# PixelStudio

### Изображение → прошивочный код за минуты

Десктопная студия для конвертации картинок в C/C++ массивы и бинарные форматы.<br/>
OLED, TFT, E-paper — без ручной рутины с hex-дампами.

<br/>

[![Скачать](https://img.shields.io/badge/Скачать-Windows_x64-0078d4?style=for-the-badge&logo=windows&logoColor=white)](https://github.com/DeeTech-Labs/PixelStudio/releases)
[![Документация](https://img.shields.io/badge/Документация-building.md-0f766e?style=for-the-badge)](docs/building.md)
[![Участие](https://img.shields.io/badge/Участие-руководство-6366f1?style=for-the-badge)](CONTRIBUTING.md)

<br/>

[![GitHub stars](https://img.shields.io/github/stars/DeeTech-Labs/PixelStudio?style=flat-square)](https://github.com/DeeTech-Labs/PixelStudio/stargazers)
[![Release](https://img.shields.io/github/v/release/DeeTech-Labs/PixelStudio?style=flat-square&label=релиз&sort=semver)](https://github.com/DeeTech-Labs/PixelStudio/releases)
[![CI](https://img.shields.io/github/actions/workflow/status/DeeTech-Labs/PixelStudio/build.yml?branch=main&style=flat-square&label=сборка)](https://github.com/DeeTech-Labs/PixelStudio/actions/workflows/build.yml)
[![License](https://img.shields.io/badge/license-MIT-blue?style=flat-square)](LICENSE)
[![Qt](https://img.shields.io/badge/Qt-6.8-41cd52?style=flat-square&logo=qt)](https://www.qt.io/)
[![C++](https://img.shields.io/badge/C++-17-00599C?style=flat-square&logo=cplusplus&logoColor=white)](https://isocpp.org/)

</div>

> [!TIP]
>
> <table>
> <tr>
> <td valign="middle">
> <h3><a href="Funding.md">🎉 Поддержать проект</a></h3>
> <p><strong><a href="https://www.donationalerts.com/r/deexsed">DonationAlerts</a></strong> — в сообщении к донату можно написать пожелания по доработкам.</p>
> </td>
> <td valign="middle" align="right" width="140">
> <a href="https://www.donationalerts.com/r/deexsed"><img src="https://files.donationalerts.com/uploads/qr/3930763/qr_eca09ac80a9cc25a80ea84284da64454.png" width="128" alt="QR-код DonationAlerts" /></a>
> </td>
> </tr>
> </table>

![PixelStudio — рабочая область студии с превью, инспектором и панелью экспорта](docs/screenshots/workspace.png)

*Студия — превью в реальном времени, инспектор параметров, экспорт в один клик*

## Возможности

<table>
<tr>
<td width="50%" valign="top">

**Растеризация под экран**

Масштаб, кроп, фильтры и дизеринг — результат сразу виден в превью.

**Профили дисплея**

Mono 1-bpp, RGB565, indexed palette, пресеты Icon / Splash / E-paper.

**Генерация кода**

Заголовки C/C++, раскладка под PROGMEM, анализ кодирования.

</td>
<td width="50%" valign="top">

**Экспорт без рутины**

Пакетный экспорт, спрайт-атлас, бинарный дамп, watch folder.

**Проекты и сессии**

Сохранение настроек студии, вкладок и параметров экспорта.

**Локализация**

Русский и английский из коробки; свои языки через JSON в AppData.

</td>
</tr>
</table>

## Как это работает

```mermaid
flowchart LR
    A["🖼 PNG · JPG · SVG"]
    B["⚙ PixelStudio"]
    C["📐 Растер"]
    D["📄 .h · .bin"]
    E["🔌 MCU"]

    A --> B --> C --> D --> E

    style A fill:#1e293b,stroke:#475569,color:#e2e8f0
    style B fill:#0f766e,stroke:#14b8a6,color:#ecfdf5
    style C fill:#1e40af,stroke:#3b82f6,color:#eff6ff
    style D fill:#6d28d9,stroke:#a78bfa,color:#f5f3ff
    style E fill:#b45309,stroke:#f59e0b,color:#fffbeb
```

## Форматы пикселей

| Формат | Применение | Дисплей |
|:-------|:-----------|:--------|
| **1-bpp mono** | Иконки, статус-бары | SSD1306 OLED |
| **RGB565** | Цветной UI, сплэши | ILI9341 TFT |
| **Indexed 4/8-bit** | Спрайты, tilemaps | Game Boy-style |
| **Custom palette** | Брендовые цвета | Любой fixed-palette |

## Быстрый старт

1. Скачайте [`PixelStudio-Setup-*.exe`](https://github.com/DeeTech-Labs/PixelStudio/releases) из релизов GitHub.
2. Установите и откройте приложение.
3. Перетащите изображение → выберите профиль → экспортируйте `.h`.

SHA256 установщика — в описании [релиза](https://github.com/DeeTech-Labs/PixelStudio/releases).

> Установщик не подписан Authenticode — Windows SmartScreen может запросить подтверждение при первом запуске.

## Пример выхода

Фрагмент `.h` для OLED 128×64 (mono, SSD1306):

```c
// Icon — 128×64
// Layout: vertical page buffer (SSD1306)

#define ICON_LOGO_WIDTH  128
#define ICON_LOGO_HEIGHT 64
static const uint8_t icon_logo[] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x3C, 0x00, 0x42, 0x00,
    0x81, 0x00, 0x81, 0x00, 0x42, 0x00, 0x3C, 0x00,
    /* … */
};
```

![Экран приветствия PixelStudio](docs/screenshots/welcome.png)

*Приветствие — недавние проекты и быстрый старт*

## Для разработчиков

<table>
<tr>
<td width="33%" align="center">

[**Сборка**](docs/building.md)

CMake · Qt 6.8 · Ninja

</td>
<td width="33%" align="center">

[**Участие**](CONTRIBUTING.md)

PR · стиль · переводы

</td>
<td width="33%" align="center">

[**Журнал изменений**](CHANGELOG.md)

История версий

</td>
</tr>
</table>

```
PixelStudio/
├── src/
│   ├── processing/   растер, палитры, codegen
│   ├── export/       пакетный экспорт, атлас, watch folder
│   └── persistence/  проекты и настройки
├── translations/     RU · EN (+ переопределения в AppData)
└── installer/        Inno Setup, CI pipeline
```

Qt 6.8 Quick · C++17 · CMake · GitHub Actions

## Сообщество

| | |
|:--|:--|
| [Сообщить об ошибке](https://github.com/DeeTech-Labs/PixelStudio/issues/new?template=bug_report.yml) | Баг с картинкой? Приложите PNG и профиль дисплея — разберём быстрее |
| [Предложить улучшение](https://github.com/DeeTech-Labs/PixelStudio/issues/new?template=feature_request.yml) | Опишите дисплей и контроллер (SSD1306, ILI9341…) |
| Уязвимости | [SECURITY.md](SECURITY.md) — private advisory |
| Кодекс | [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md) |

> При баг-репорте: логи в **Настройки → Данные → Открыть папку логов**.

## Благодарность сообществу

PixelStudio развивается благодаря всем, кто сообщает об ошибках, предлагает идеи и присылает pull request'ы.

[![Контрибьюторы PixelStudio](https://contrib.rocks/image?repo=DeeTech-Labs/PixelStudio&columns=8)](https://github.com/DeeTech-Labs/PixelStudio/graphs/contributors)

Хотите попасть в этот список? См. [руководство для участников](CONTRIBUTING.md).

Если PixelStudio полезен — поставьте ⭐ репозиторию.

[![Star History](https://api.star-history.com/svg?repos=DeeTech-Labs/PixelStudio&type=Date)](https://star-history.com/#DeeTech-Labs/PixelStudio&Date)

---

**[MIT License](LICENSE)** · [NOTICE.txt](NOTICE.txt)

*Сделано на Qt · Windows x64*
