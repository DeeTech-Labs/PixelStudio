#!/usr/bin/env bash
# macOS: cmake Release + macdeployqt + prune → installer/staging-macos/PixelStudio.app
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-${ROOT}/build/Release}"
QT_DIR="${QT_DIR:-${QT_ROOT_DIR:-}}"
STAGE_DIR="${ROOT}/installer/staging-macos"
QML_DIR="${ROOT}/src/qml"
SKIP_BUILD="${SKIP_BUILD:-false}"
SKIP_DEPLOY="${SKIP_DEPLOY:-false}"
SKIP_PRUNE="${SKIP_PRUNE:-false}"

# shellcheck source=../cmake/read-pixel-studio-version.sh
source "${ROOT}/cmake/read-pixel-studio-version.sh"
# shellcheck source=macos-deploy-prune.sh
source "${ROOT}/installer/macos-deploy-prune.sh"

if [[ "$(uname -s)" != "Darwin" ]]; then
    echo "macos-build-bundle.sh requires macOS." >&2
    exit 1
fi

if [[ -z "$QT_DIR" ]]; then
    echo "Set QT_DIR or QT_ROOT_DIR to the Qt 6.8 clang kit root." >&2
    exit 1
fi

MACDEPLOYQT="${QT_DIR}/bin/macdeployqt"
if [[ ! -x "$MACDEPLOYQT" ]]; then
    echo "macdeployqt not found: $MACDEPLOYQT" >&2
    exit 1
fi

read_pixel_studio_version "$ROOT"
echo "Version: ${PS_VERSION_DISPLAY} (tag ${PS_VERSION_TAG})"

chmod +x "${ROOT}/installer/macos-make-icns.sh"
"${ROOT}/installer/macos-make-icns.sh"

MACOS_ARCHITECTURES="${MACOS_ARCHITECTURES:-arm64;x86_64}"

if [[ "$SKIP_BUILD" != "true" ]]; then
    cmake -S "$ROOT" -B "$BUILD_DIR" -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_PREFIX_PATH="$QT_DIR" \
        -DCMAKE_OSX_ARCHITECTURES="$MACOS_ARCHITECTURES"
    cmake --build "$BUILD_DIR"
fi

APP_BUNDLE="${BUILD_DIR}/PixelStudio.app"
if [[ ! -d "$APP_BUNDLE" ]]; then
    echo "Missing app bundle: $APP_BUNDLE" >&2
    exit 1
fi

if [[ "$SKIP_DEPLOY" != "true" ]]; then
    "$MACDEPLOYQT" "$APP_BUNDLE" \
        -qmldir="$QML_DIR" \
        -always-overwrite \
        -no-strip \
        -no-translations \
        -skip-plugin-types=qmltooling
fi

rm -rf "$STAGE_DIR"
mkdir -p "$STAGE_DIR"
ditto "$APP_BUNDLE" "${STAGE_DIR}/PixelStudio.app"

invoke_deploy_macos_prune "${STAGE_DIR}/PixelStudio.app" "$SKIP_PRUNE"
remove_deploy_macos_empty_dirs "${STAGE_DIR}/PixelStudio.app"

chmod +x "${ROOT}/installer/macos-verify-universal.sh"
"${ROOT}/installer/macos-verify-universal.sh" "${STAGE_DIR}/PixelStudio.app"

staged_files="$(find "$STAGE_DIR" -type f | wc -l | tr -d ' ')"
staged_mb="$(du -sm "$STAGE_DIR" | awk '{print $1}')"
echo "macOS bundle payload: ${staged_files} file(s), ~${staged_mb} MB"
echo "Staged: ${STAGE_DIR}/PixelStudio.app"
