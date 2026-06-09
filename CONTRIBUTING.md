# Участие в разработке PixelStudio

> **English:** [docs/en/CONTRIBUTING.md](docs/en/CONTRIBUTING.md)

Спасибо за интерес к проекту. Ниже — минимум для сборки, проверки и отправки изменений.

## Перед началом

- Проверьте [существующие issues](https://github.com/DeeTech-Labs/PixelStudio/issues).
- Крупные изменения лучше согласовать в issue заранее.
- Один PR — одна логическая задача; избегайте несвязанных правок.

## Окружение

| Инструмент | Заметки |
|------------|---------|
| Windows 10/11 x64 | Основная платформа |
| Qt 6.8.x MSVC 2022 64-bit | Quick, QuickControls2, Gui, Network, Concurrent, Svg |
| CMake ≥ 3.16 | Рекомендуется Ninja |
| Visual Studio 2022 | C++ desktop workload |
| Inno Setup 6 | Только для сборки установщика |

Подробная сборка: [docs/building.md](docs/building.md).

## Сборка

```powershell
$qt = "C:\Qt6\6.8.2\msvc2022_64"

cmake -S . -B build\Release -G Ninja `
  -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_PREFIX_PATH="$qt"

cmake --build build\Release
```

Запуск: `build\Release\PixelStudio.exe`. Для redistributable — `windeployqt` или `installer\build-installer.ps1`.

## Версии

Схема **`Major.Minor.Patch`** + опциональный **fix** (`0.1.0b`, `1.2.3b`): [docs/versioning.md](docs/versioning.md).

Правки в [`cmake/PixelStudioVersion.cmake`](cmake/PixelStudioVersion.cmake) и [`CHANGELOG.md`](CHANGELOG.md) (`[Unreleased]`).  
На PR — метки `major` / `minor` / `patch` / `version-fix` для Release Drafter.

## Стиль кода

- [.editorconfig](.editorconfig): UTF-8, LF, 4 пробела для C++/QML.
- Сохраняйте границы модулей (`src/processing`, `src/export`, …).
- Логику по возможности добавляйте в `pixelstudio_core`.

## Переводы

Переводы в JSON: `translations/en.json`, `translations/ru.json`. Код и название языка — в самом файле (`language`, `name`).

```json
{
  "language": "ru",
  "name": "Русский",
  "author": "Имя основного переводчика",
  "contributors": ["Кто помогал", "Кто правил строки"],
  "contexts": {
    "Shell": { "&File": "&Файл" },
    "Core": { "Home": "Главная" }
  }
}
```

- `author` — основной автор перевода (отображается в настройках).
- `contributors` — массив имён тех, кто помогал или вносил правки.

Контексты: `Welcome`, `Inspector`, `Shell`, `Workspace`, `Controls`, `Core`. В QML — `pragma Translator: <Контекст>` и `qsTr()`.

**Пользовательские языки** (без пересборки): положите `%AppData%/DeeTech/PixelStudio/translations/<код>.json` — язык появится в меню автоматически. Пример: `translations/de.example.json`.

Встроенный язык: добавьте `translations/<код>.json` (два символа в имени файла), пересоберите.

## Pull request

1. Форк, ветка от `main`.
2. Локальная сборка на Windows (CI — `build.yml`).
3. `CHANGELOG.md` → `[Unreleased]` для пользовательских изменений.
4. Заполните шаблон PR (на русском).
5. Скриншоты в `docs/screenshots/` при заметных изменениях UI.

## Релизы (maintainers)

| Событие | Workflow |
|---------|----------|
| PR / push в `main` (код) | `build.yml` |
| Tag `v0.1.0` | `release.yml` — установщик + Release |
| Вручную | Actions → Release → Run workflow |

Метки: [`.github/labels.yml`](.github/labels.yml). Шаблоны issues: [`.github/ISSUE_TEMPLATE/`](.github/ISSUE_TEMPLATE/) (русский).

## Кодекс поведения

[CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md)

Неприемлемое поведение — через Issues или [SECURITY.md](SECURITY.md), если уместно.
