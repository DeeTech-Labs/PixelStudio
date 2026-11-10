#!/usr/bin/env bash
# Prune unused Qt modules from a macdeployqt staging tree (inside .app bundle).
set -euo pipefail

deploy_macos_prune_groups() {
    cat <<'EOF'
Unused Quick Controls styles (Imagine)|Contents/Resources/qml/QtQuick/Controls/Imagine|Contents/Frameworks/QtQuickControls2Imagine.framework|Contents/Frameworks/QtQuickControls2ImagineStyleImpl.framework
Unused Quick Controls styles (Universal)|Contents/Resources/qml/QtQuick/Controls/Universal|Contents/Frameworks/QtQuickControls2Universal.framework|Contents/Frameworks/QtQuickControls2UniversalStyleImpl.framework
Unused Quick Controls styles (Material)|Contents/Resources/qml/QtQuick/Controls/Material|Contents/Frameworks/QtQuickControls2Material.framework|Contents/Frameworks/QtQuickControls2MaterialStyleImpl.framework
Unused Quick Controls styles (FluentWinUI3)|Contents/Resources/qml/QtQuick/Controls/FluentWinUI3|Contents/Frameworks/QtQuickControls2FluentWinUI3.framework|Contents/Frameworks/QtQuickControls2FluentWinUI3StyleImpl.framework
Unused Quick Controls styles (Windows)|Contents/Resources/qml/QtQuick/Controls/Windows|Contents/Frameworks/QtQuickControls2WindowsStyleImpl.framework
Dialog QML variants for unused styles|Contents/Resources/qml/QtQuick/Dialogs/quickimpl/qml/+Imagine|Contents/Resources/qml/QtQuick/Dialogs/quickimpl/qml/+Material|Contents/Resources/qml/QtQuick/Dialogs/quickimpl/qml/+Universal
Unused image format plugins|Contents/PlugIns/imageformats/libqicns.dylib|Contents/PlugIns/imageformats/libqjp2.dylib|Contents/PlugIns/imageformats/libqpdf.dylib|Contents/PlugIns/imageformats/libqtga.dylib|Contents/PlugIns/imageformats/libqwbmp.dylib
Qt Quick 3D (not used)|Contents/Frameworks/QtQuick3D.framework|Contents/Frameworks/QtQuick3DAssetImport.framework|Contents/Frameworks/QtQuick3DEffects.framework|Contents/Frameworks/QtQuick3DHelpers.framework|Contents/Frameworks/QtQuick3DHelpersImpl.framework|Contents/Frameworks/QtQuick3DParticleEffects.framework|Contents/Frameworks/QtQuick3DParticles.framework|Contents/Frameworks/QtQuick3DRuntimeRender.framework|Contents/Frameworks/QtQuick3DUtils.framework|Contents/Resources/qml/QtQuick3D
Unused modules (PDF, virtual keyboard)|Contents/Frameworks/QtPdf.framework|Contents/Frameworks/QtVirtualKeyboard.framework|Contents/Resources/qml/QtQuick/Pdf
Unused Qt Quick Shapes|Contents/Frameworks/QtQuickShapes.framework|Contents/Resources/qml/QtQuick/Shapes
Unused Qt Quick NativeStyle|Contents/Resources/qml/QtQuick/NativeStyle
Unused touch input plugin|Contents/PlugIns/generic/libqtuiotouchplugin.dylib
EOF
}

deploy_macos_path_bytes() {
    local path="$1"
    if [[ ! -e "$path" ]]; then
        echo 0
        return
    fi
    if [[ -d "$path" ]]; then
        du -sk "$path" | awk '{print $1 * 1024}'
    else
        stat -f%z "$path"
    fi
}

invoke_deploy_macos_prune() {
    local app_bundle="$1"
    local skip_prune="${2:-false}"

    if [[ "$skip_prune" == "true" ]]; then
        echo "Prune skipped."
        return
    fi

    local removed_groups=0
    local saved_bytes=0

    while IFS= read -r line; do
        [[ -z "$line" ]] && continue
        local name="${line%%|*}"
        local paths_part="${line#*|}"
        IFS='|' read -ra paths <<< "$paths_part"

        local group_paths=()
        local bytes_before=0

        for rel in "${paths[@]}"; do
            local full="${app_bundle}/${rel}"
            if [[ -e "$full" ]]; then
                group_paths+=("$full")
                bytes_before=$((bytes_before + $(deploy_macos_path_bytes "$full")))
            fi
        done

        if [[ ${#group_paths[@]} -eq 0 ]]; then
            continue
        fi

        for full in "${group_paths[@]}"; do
            rm -rf "$full"
        done

        removed_groups=$((removed_groups + 1))
        saved_bytes=$((saved_bytes + bytes_before))
        printf '  pruned: %s (%s bytes)\n' "$name" "$bytes_before"
    done < <(deploy_macos_prune_groups)

    local saved_mb
    saved_mb="$(awk "BEGIN {printf \"%.2f\", ${saved_bytes}/1048576}")"
    printf 'Prune done: %s group(s), ~%s MB removed.\n' "$removed_groups" "$saved_mb"
}

remove_deploy_macos_empty_dirs() {
    local root_dir="$1"
    local removed=0
    local found=true

    while $found; do
        found=false
        while IFS= read -r -d '' dir; do
            if [[ -z "$(ls -A "$dir" 2>/dev/null)" ]]; then
                rmdir "$dir" 2>/dev/null && removed=$((removed + 1)) && found=true
            fi
        done < <(find "$root_dir" -type d -depth -print0 2>/dev/null)
    done

    if [[ $removed -gt 0 ]]; then
        echo "Removed $removed empty director(ies)."
    fi
}
