# Версионирование

> **English:** [en/versioning.md](en/versioning.md)

## Формат

**`G.M.m`** или **`G.M.m` + fix**, например:

| Отображение | CMake (`PixelStudioVersion.cmake`) |
|-------------|-----------------------------------|
| `0.1.0` | `GLOBAL=0`, `MAJOR=1`, `MINOR=0`, `FIX=""` |
| `0.1.0b` | `GLOBAL=0`, `MAJOR=1`, `MINOR=0`, `FIX="b"` |
| `1.2.3b` | `GLOBAL=1`, `MAJOR=2`, `MINOR=3`, `FIX="b"` |

- **G** — `PS_VERSION_GLOBAL`
- **M** — `PS_VERSION_MAJOR`
- **m** — `PS_VERSION_MINOR`
- **fix** — `PS_VERSION_FIX` (одна латинская буква/символ в cmake, без точки перед ней)

Единый источник правды: [`cmake/PixelStudioVersion.cmake`](../cmake/PixelStudioVersion.cmake).  
Теги Git и установщик: `v0.1.0`, `v1.2.3b`.

## Метки PR и Release Drafter

На PR вешайте **одну** метку уровня версии (если релиз планируется):

| Метка | Semver (черновик Drafter) | CMake |
|-------|---------------------------|--------|
| `patch` | +0.0.1 (по умолчанию) | `PS_VERSION_MINOR++`, `FIX=""` |
| `minor` | +0.1.0 | `PS_VERSION_MAJOR++`, `MINOR=0`, `FIX=""` |
| `major` | +1.0.0 | `PS_VERSION_GLOBAL++`, `MAJOR=0`, `MINOR=0`, `FIX=""` |
| `version-fix` | **не меняет** цифры в Drafter | только `PS_VERSION_FIX` (например `"b"`) |

Соответствие semver ↔ `G.M.m`: черновик `v$RESOLVED_VERSION` от [Release Drafter](../.github/workflows/release-drafter.yml) — это **три числа**; перед релизом сверьте и обновите cmake под фактическую схему.

### Только fix (`1.2.3` → `1.2.3b`)

1. Метка PR: `version-fix`
2. В cmake: выставить `PS_VERSION_FIX "b"` (цифры не трогать)
3. `CHANGELOG.md` → `[Unreleased]`
4. Тег и релиз: **`v1.2.3b`** (не полагаться на авто-тег Drafter)
5. Push тега → workflow [Release](../.github/workflows/release.yml)

## Публикация релиза

1. Черновик в GitHub Releases (Release Drafter) или ручной тег
2. Актуальный [`cmake/PixelStudioVersion.cmake`](../cmake/PixelStudioVersion.cmake)
3. `git tag v1.2.3b` → `git push origin v1.2.3b`
4. CI Release соберёт `PixelStudio-Setup-<version>.exe`
