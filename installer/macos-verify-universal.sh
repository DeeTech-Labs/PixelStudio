#!/usr/bin/env bash
# Verify PixelStudio.app (and deployed QtCore) are fat binaries: arm64 + x86_64.
set -euo pipefail

APP_BUNDLE="${1:?Usage: macos-verify-universal.sh <path/to/PixelStudio.app>}"

if [[ "$(uname -s)" != "Darwin" ]]; then
    echo "macos-verify-universal.sh requires macOS." >&2
    exit 1
fi

if [[ ! -d "$APP_BUNDLE" ]]; then
    echo "App bundle not found: $APP_BUNDLE" >&2
    exit 1
fi

EXE="${APP_BUNDLE}/Contents/MacOS/PixelStudio"
if [[ ! -f "$EXE" ]]; then
    echo "Executable not found: $EXE" >&2
    exit 1
fi

require_universal() {
    local binary="$1"
    local label="$2"
    local info=""
    if ! info="$(lipo -info "$binary" 2>&1)"; then
        echo "lipo failed for $label ($binary): $info" >&2
        return 1
    fi
    echo "$label: $info"
    [[ "$info" == *"arm64"* ]] || {
        echo "Missing arm64 in $label ($binary)" >&2
        return 1
    }
    [[ "$info" == *"x86_64"* ]] || {
        echo "Missing x86_64 in $label ($binary)" >&2
        return 1
    }
}

require_universal "$EXE" "PixelStudio"

QT_CORE="$(find "${APP_BUNDLE}/Contents/Frameworks" -path '*/QtCore.framework/*/QtCore' -type f 2>/dev/null | head -n 1 || true)"
if [[ -n "$QT_CORE" ]]; then
    require_universal "$QT_CORE" "QtCore"
else
    echo "QtCore.framework not deployed yet — skipping framework check"
fi

echo "Universal binary check passed (arm64 + x86_64)."
