# Roadmap PixelStudio

[← README](README.md) · [Changelog](CHANGELOG.md) · [Участие](CONTRIBUTING.md)

Публичный план развития. Текущая версия: **0.2.1** (см. [`cmake/PixelStudioVersion.cmake`](cmake/PixelStudioVersion.cmake)).

## Как влиять на приоритеты

| Способ | Когда использовать |
|--------|-------------------|
| [Issue «Предложить улучшение»](https://github.com/DeeTech-Labs/PixelStudio/issues/new?template=feature_request.yml) | Конкретная фича с описанием сценария |
| [Discussions → Ideas](https://github.com/DeeTech-Labs/PixelStudio/discussions/new?category=ideas) | Обсуждение и голосование 👍 |
| [Issue «Вклад сообщества»](https://github.com/DeeTech-Labs/PixelStudio/issues/new?template=community_contribution.yml) | Готовый пресет, перевод, пример |

Issue с меткой `roadmap` попадают в этот документ при планировании релизов.

## Сейчас в работе

_Пока нет открытых пунктов с меткой `roadmap` в активной разработке._

Следите за [milestone](https://github.com/DeeTech-Labs/PixelStudio/milestones) и `[Unreleased]` в [CHANGELOG.md](CHANGELOG.md).

## Запланировано

Направления ближайших итераций (без жёстких дат):

1. **Профили дисплеев** — больше готовых пресетов под популярные контроллеры (SSD1306, ILI9341, ST7789).
2. **Экспорт** — удобнее пакетный экспорт, atlas и watch folder для типовых embedded-пайплайнов.
3. **Качество превью** — точнее соответствие результата на реальном железе (дизеринг, палитры).
4. **Сообщество** — переводы, примеры проектов, шаблоны под конкретные платы.

## Идеи (backlog)

Смотрите открытые issue с метками [`enhancement`](https://github.com/DeeTech-Labs/PixelStudio/issues?q=is%3Aissue+is%3Aopen+label%3Aenhancement) и [`roadmap`](https://github.com/DeeTech-Labs/PixelStudio/issues?q=is%3Aissue+is%3Aopen+label%3Aroadmap), а также [Discussions](https://github.com/DeeTech-Labs/PixelStudio/discussions).

## Вне scope (пока)

- Сборка и официальная поддержка macOS / Linux (только Windows в релизах).
- Онлайн-сервис или облачная конвертация.
- Редактор прошивки / полный IDE для MCU.

## Версионирование

Релизы — [SemVer](https://semver.org/lang/ru/). Процесс публикации: [docs/versioning.md](docs/versioning.md).

> **Для мэйнтейнеров:** включите [GitHub Discussions](https://github.com/DeeTech-Labs/PixelStudio/settings) и создайте категорию **Ideas** (slug `ideas`) — для шаблона `.github/DISCUSSION_TEMPLATE/ideas.yml` и ссылки в меню issue.

---

[← README](README.md)
