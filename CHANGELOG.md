# История изменений

Формат основан на [Keep a Changelog](https://keepachangelog.com/ru/1.1.0/),
версии — [Semantic Versioning](https://semver.org/lang/ru/).

## [Unreleased]

### Добавлено

- Документация на русском и английском (`docs/`, `docs/en/`), CI/CD, шаблоны issues.

### Изменено

- Каталоги и API переводов переименованы: `translations/`, `src/translation/`, `%AppData%/…/translations/` (миграция с прежнего `i18n/` при первом запуске).
- Переводы в JSON (`translations/en.json`, `translations/ru.json`); пользовательские языки и правки — в `%AppData%/DeeTech/PixelStudio/translations/` без пересборки.

## [0.1.0] - 2026-06-04

### Добавлено

- Десктопное приложение: изображения → C/C++ массивы для OLED/TFT.
- UI на Qt 6 Quick: welcome, студия с вкладками, панели инспектора.
- Профили дисплея, растеризация, генерация кода.
- Пакетный экспорт, sprite atlas, watch folder, импорт заголовков.
- Интерфейс на русском и английском (`translations/`).
- Сборка установщика Windows (`installer/build-installer.ps1`, Inno Setup).

[Unreleased]: https://github.com/DeeTech-Labs/PixelStudio/compare/v0.1.0...HEAD
[0.1.0]: https://github.com/DeeTech-Labs/PixelStudio/releases/tag/v0.1.0
