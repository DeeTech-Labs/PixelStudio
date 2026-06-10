#!/usr/bin/env bash
# Parses cmake/PixelStudioVersion.cmake — bash reader for macOS CI/release tooling.
set -euo pipefail

read_pixel_studio_version() {
    local repo_root="${1:-}"
    if [[ -z "$repo_root" ]]; then
        repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
    fi

    local cmake_file="${repo_root}/cmake/PixelStudioVersion.cmake"
    if [[ ! -f "$cmake_file" ]]; then
        echo "Version file not found: $cmake_file" >&2
        return 1
    fi

    local content major minor patch fix display info tag
    content="$(<"$cmake_file")"

    if [[ ! "$content" =~ set\(PS_VERSION_MAJOR[[:space:]]+([0-9]+)\) ]]; then
        echo "PS_VERSION_MAJOR not found in $cmake_file" >&2
        return 1
    fi
    major="${BASH_REMATCH[1]}"

    if [[ ! "$content" =~ set\(PS_VERSION_MINOR[[:space:]]+([0-9]+)\) ]]; then
        echo "PS_VERSION_MINOR not found in $cmake_file" >&2
        return 1
    fi
    minor="${BASH_REMATCH[1]}"

    if [[ ! "$content" =~ set\(PS_VERSION_PATCH[[:space:]]+([0-9]+)\) ]]; then
        echo "PS_VERSION_PATCH not found in $cmake_file" >&2
        return 1
    fi
    patch="${BASH_REMATCH[1]}"

    if [[ ! "$content" =~ set\(PS_VERSION_FIX[[:space:]]+\"([^\"]*)\"\) ]]; then
        echo "PS_VERSION_FIX not found in $cmake_file" >&2
        return 1
    fi
    fix="${BASH_REMATCH[1]}"

    if [[ -n "$fix" ]]; then
        display="${major}.${minor}.${patch}${fix}"
    else
        display="${major}.${minor}.${patch}"
    fi
    info="${major}.${minor}.${patch}.0"
    tag="v${display}"

    PS_VERSION_MAJOR="$major"
    PS_VERSION_MINOR="$minor"
    PS_VERSION_PATCH="$patch"
    PS_VERSION_FIX="$fix"
    PS_VERSION_DISPLAY="$display"
    PS_VERSION_INFO="$info"
    PS_VERSION_TAG="$tag"
}

test_pixel_studio_release_tag() {
    local tag="${1:?tag required}"
    local repo_root="${2:-}"

    read_pixel_studio_version "$repo_root"

    if [[ "$tag" != "$PS_VERSION_TAG" ]]; then
        echo "Git tag '$tag' does not match cmake version '$PS_VERSION_TAG' (cmake/PixelStudioVersion.cmake)." >&2
        return 1
    fi
}

if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    read_pixel_studio_version "${1:-}"
    printf 'Display: %s\n' "$PS_VERSION_DISPLAY"
    printf 'Tag:     %s\n' "$PS_VERSION_TAG"
    printf 'Info:    %s\n' "$PS_VERSION_INFO"
fi
