#!/usr/bin/env bash
# Create unsigned DMG from installer/staging-macos/PixelStudio.app
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
STAGE_APP="${ROOT}/installer/staging-macos/PixelStudio.app"
OUTPUT_DIR="${ROOT}/installer/output"
DMG_VARIANT="${DMG_VARIANT:-macos_universal}"

# shellcheck source=../cmake/read-pixel-studio-version.sh
source "${ROOT}/cmake/read-pixel-studio-version.sh"

if [[ "$(uname -s)" != "Darwin" ]]; then
    echo "macos-make-dmg.sh requires macOS." >&2
    exit 1
fi

if [[ ! -d "$STAGE_APP" ]]; then
    echo "Staged app not found: $STAGE_APP (run macos-build-bundle.sh first)" >&2
    exit 1
fi

read_pixel_studio_version "$ROOT"
DMG_NAME="PixelStudio_${PS_VERSION_DISPLAY}_${DMG_VARIANT}.dmg"
DMG_PATH="${OUTPUT_DIR}/${DMG_NAME}"
TEMP_DMG="${OUTPUT_DIR}/.PixelStudio-temp.dmg"

mkdir -p "$OUTPUT_DIR"
rm -f "$DMG_PATH" "$TEMP_DMG"

hdiutil create -volname "PixelStudio" -srcfolder "$STAGE_APP" -ov -format UDRW -fs HFS+ "$TEMP_DMG"
hdiutil convert "$TEMP_DMG" -format UDZO -imagekey zlib-level=9 -o "$DMG_PATH"
rm -f "$TEMP_DMG"

echo "Wrote $DMG_PATH"
