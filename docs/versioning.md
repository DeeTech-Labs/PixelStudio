# Версионирование

[← README](../README.md)

PixelStudio следует [Semantic Versioning 2.0.0](https://semver.org/lang/ru/): `MAJOR.MINOR.PATCH[-prerelease]`.

## Единый источник

Версия задаётся в одном файле:

[`cmake/PixelStudioVersion.cmake`](../cmake/PixelStudioVersion.cmake)

```cmake
set(PS_VERSION_MAJOR 0)
set(PS_VERSION_MINOR 2)
set(PS_VERSION_PATCH 1)
set(PS_VERSION_PRERELEASE "")   # пусто = стабильный релиз
```

Отсюда автоматически берутся:

- строка в «О программе» и заголовке окна;
- Git-тег релиза (`v0.2.1`, `v0.3.0-rc.1`, …);
- имя установщика `PixelStudio-Setup-<версия>.exe`;
- проверка CI при публикации release.

Проверить текущую версию локально:

```powershell
.\cmake\ReadPixelStudioVersion.ps1
```

## Когда увеличивать номер

| Изменение | Версия | Пример |
|-----------|--------|--------|
| Обратно несовместимые изменения публичного поведения | **MAJOR** | `0.2.1` → `1.0.0` |
| Новая функциональность без поломки совместимости | **MINOR** | `0.2.1` → `0.3.0` |
| Исправления багов | **PATCH** | `0.2.1` → `0.2.2` |

Пока проект на **0.y.z**, публичный API считается нестабильным ([SemVer §4](https://semver.org/lang/ru/#spec-semver)).

## Предрелизные версии

Нестабильные сборки обозначаются **через дефис**, не суффиксом к патчу:

| Статус | `PS_VERSION_PRERELEASE` | Тег | Отображение |
|--------|-------------------------|-----|-------------|
| Стабильный | `""` | `v0.2.1` | `0.2.1` |
| Бета | `"beta"` | `v0.3.0-beta` | `0.3.0-beta` |
| Бета с номером | `"beta.2"` | `v0.3.0-beta.2` | `0.3.0-beta.2` |
| Release candidate | `"rc.1"` | `v0.3.0-rc.1` | `0.3.0-rc.1` |

Идентификаторы — только `[0-9A-Za-z-]`, разделённые точками; числовые идентификаторы без ведущих нулей.

## Публикация релиза

1. Обновите `cmake/PixelStudioVersion.cmake`.
2. Перенесите записи из `[Unreleased]` в новую секцию [`CHANGELOG.md`](../CHANGELOG.md).
3. Закоммитьте, создайте и запушьте тег **точно** как `PS_VERSION_TAG` (например `v0.2.2`):

   ```powershell
   git tag v0.2.2
   git push origin v0.2.2
   ```

4. Workflow [release.yml](../.github/workflows/release.yml) соберёт установщик и опубликует GitHub Release.

Тег должен совпадать с cmake — иначе CI завершится с ошибкой.

## Исторические теги

Релиз `v0.2.0b` (2026-06-09) опубликован до перехода на SemVer-предрелизы; в CHANGELOG он остаётся как `[0.2.0b]`. Новые предрелизы используют формат `-beta`, `-rc.1` и т.д.

---

[← README](../README.md) · [Сборка](building.md)
