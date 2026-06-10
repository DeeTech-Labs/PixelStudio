# Участие в разработке

[← README](README.md) · [Сборка](docs/building.md) · [Issues](https://github.com/DeeTech-Labs/PixelStudio/issues)

Спасибо за интерес к PixelStudio. Ниже — правила и ориентиры для контрибьюторов.

## Содержание

- [Перед началом](#перед-началом)
- [Сборка](#сборка)
- [Стиль кода](#стиль-кода)
- [Переводы](#переводы)
- [Pull request](#pull-request)
- [Кодекс поведения](#кодекс-поведения)

## Перед началом

1. Проверьте [существующие issues](https://github.com/DeeTech-Labs/PixelStudio/issues).
2. Крупные изменения согласуйте в issue заранее.
3. Один PR — одна логическая задача, без несвязанных правок.

## Сборка

Полная инструкция — [docs/building.md](docs/building.md).

Перед отправкой PR соберите проект локально на Windows (Release). Чеклист — в [шаблоне PR](.github/pull_request_template.md).

## Стиль кода

| Правило | Детали |
|---------|--------|
| Форматирование | [.editorconfig](.editorconfig) — UTF-8, LF, 4 пробела для C++/QML |
| Модули | Сохраняйте границы `src/processing`, `src/export`, … |
| Ядро | Логику по возможности добавляйте в `pixelstudio_core` |

## Переводы

Файлы: `translations/en.json`, `translations/ru.json`. Код и название языка — в самом файле (`language`, `name`).

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

| Поле | Назначение |
|------|------------|
| `author` | Основной автор (отображается в настройках) |
| `contributors` | Помощники и редакторы строк |
| `contexts` | Группы строк: `Welcome`, `Inspector`, `Shell`, `Workspace`, `Controls`, `Core` |

В QML: `pragma Translator: <Контекст>` и `qsTr()`.

**Пользовательский язык без пересборки** — `%AppData%/DeeTech/PixelStudio/translations/<код>.json` (пример структуры: `translations/de.example.json`).

**Встроенный язык** — `translations/<код>.json` (два символа в имени), пересборка.

## Pull request

1. Форк → ветка от `main`.
2. Заполните [шаблон PR](.github/pull_request_template.md) на русском.

## Кодекс поведения

Участники следуют [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md).

Неприемлемое поведение — через [Issues](https://github.com/DeeTech-Labs/PixelStudio/issues) или [SECURITY.md](SECURITY.md), если затрагивает безопасность.

---

[← README](README.md) · [Сборка](docs/building.md)
