# Версионирование

> **English:** [en/versioning.md](en/versioning.md)

## Формат

**`Major.Minor.Patch`** или **`Major.Minor.Patch` + fix**, например:

| Отображение | CMake (`PixelStudioVersion.cmake`) |
|-------------|-----------------------------------|
| `0.1.0` | `MAJOR=0`, `MINOR=1`, `PATCH=0`, `FIX=""` |
| `0.1.0b` | `MAJOR=0`, `MINOR=1`, `PATCH=0`, `FIX="b"` |
| `1.2.3b` | `MAJOR=1`, `MINOR=2`, `PATCH=3`, `FIX="b"` |

- **Major** — `PS_VERSION_MAJOR`
- **Minor** — `PS_VERSION_MINOR`
- **Patch** — `PS_VERSION_PATCH`
- **fix** — `PS_VERSION_FIX` (одна латинская буква/символ в cmake, без точки перед ней)

Единый источник правды: [`cmake/PixelStudioVersion.cmake`](../cmake/PixelStudioVersion.cmake).  
Теги Git и установщик: `v0.1.0`, `v1.2.3b`.

PowerShell: `.\cmake\ReadPixelStudioVersion.ps1`

## Метки PR и Release Drafter

На PR вешайте **одну** метку уровня версии (если релиз планируется):

| Метка | Semver (черновик Drafter) | CMake |
|-------|---------------------------|--------|
| `patch` | +0.0.1 (по умолчанию) | `PS_VERSION_PATCH++`, `FIX=""` |
| `minor` | +0.1.0 | `PS_VERSION_MINOR++`, `PATCH=0`, `FIX=""` |
| `major` | +1.0.0 | `PS_VERSION_MAJOR++`, `MINOR=0`, `PATCH=0`, `FIX=""` |
| `version-fix` | **не меняет** цифры в Drafter | только `PS_VERSION_FIX` (например `"b"`) |

Черновик `v$RESOLVED_VERSION` от [Release Drafter](../.github/workflows/release-drafter.yml) совпадает с semver; перед релизом обновите cmake.

### Только fix (`1.2.3` → `1.2.3b`)

1. Метка PR: `version-fix`
2. В cmake: выставить `PS_VERSION_FIX "b"` (цифры не трогать)
3. `CHANGELOG.md` → `[Unreleased]`
4. Тег и релиз: **`v1.2.3b`**
5. Push тега → workflow [Release](../.github/workflows/release.yml)

## Публикация релиза

1. Черновик в GitHub Releases (Release Drafter) или ручной тег
2. Актуальный [`cmake/PixelStudioVersion.cmake`](../cmake/PixelStudioVersion.cmake)
3. `git tag v1.2.3` → `git push origin v1.2.3`
4. CI Release соберёт `PixelStudio-Setup-<version>.exe` (тег должен совпадать с cmake)
