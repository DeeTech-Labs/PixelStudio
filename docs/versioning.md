# Версионирование

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

## Метки PR

На PR вешайте **одну** метку уровня версии (если релиз планируется):

| Метка | Semver | CMake |
|-------|--------|--------|
| `patch` | +0.0.1 (по умолчанию) | `PS_VERSION_PATCH++`, `FIX=""` |
| `minor` | +0.1.0 | `PS_VERSION_MINOR++`, `PATCH=0`, `FIX=""` |
| `major` | +1.0.0 | `PS_VERSION_MAJOR++`, `MINOR=0`, `PATCH=0`, `FIX=""` |
| `version-fix` | только суффикс | только `PS_VERSION_FIX` (например `"b"`) |

Перед релизом обновите cmake вручную по таблице выше.

### Только fix (`1.2.3` → `1.2.3b`)

1. В cmake: выставить `PS_VERSION_FIX "b"` (цифры не трогать)
2. `CHANGELOG.md` → `[Unreleased]`
3. Тег и релиз: **`v1.2.3b`**
4. Push тега → workflow [Release](../.github/workflows/release.yml)

## Публикация релиза

1. Актуальный [`cmake/PixelStudioVersion.cmake`](../cmake/PixelStudioVersion.cmake)
2. `git tag v1.2.3` → `git push origin v1.2.3`
3. CI Release соберёт `PixelStudio-Setup-<version>.exe` (тег должен совпадать с cmake); заметки — `CHANGELOG.md` и автогенерация GitHub
