# Installer / packaging

Общий каталог вывода: `output/` (`.exe` и `.dmg`).

## Windows

| Файл | Назначение |
|------|------------|
| `windows-build-installer.ps1` | Полный пайплайн: сборка → windeployqt → staging → Inno Setup |
| `windows-deploy-prune.ps1` | Prune runtime (dot-source из build-скрипта) |
| `windows-PixelStudio.iss` | Скрипт Inno Setup |
| `staging-windows/` | Промежуточный deploy перед упаковкой |

```powershell
.\installer\windows-build-installer.ps1 -QtDir "C:\Qt6\6.8.2\msvc2022_64"
```

## macOS

| Файл | Назначение |
|------|------------|
| `macos-build-bundle.sh` | Сборка → macdeployqt → prune → `staging-macos/PixelStudio.app` |
| `macos-deploy-prune.sh` | Prune Qt-модулей в bundle |
| `macos-make-icns.sh` | Иконка `.icns` из PNG |
| `macos-verify-universal.sh` | Проверка fat binary (arm64 + x86_64) |
| `macos-make-dmg.sh` | DMG из staged `.app` |
| `staging-macos/` | Промежуточный deploy перед DMG |

```bash
export QT_DIR="$HOME/Qt/6.8.2/macos"
installer/macos-build-bundle.sh
installer/macos-make-dmg.sh
```
