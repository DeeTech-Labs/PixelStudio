# Единый источник версии PixelStudio — SemVer 2.0.0 (https://semver.org/lang/ru/)
# Формат: MAJOR.MINOR.PATCH[-prerelease]
# Примеры prerelease: beta, beta.1, rc.1
set(PS_VERSION_MAJOR 0)
set(PS_VERSION_MINOR 2)
set(PS_VERSION_PATCH 1)
set(PS_VERSION_PRERELEASE "")

if(PS_VERSION_PRERELEASE STREQUAL "")
    set(PS_VERSION_DISPLAY "${PS_VERSION_MAJOR}.${PS_VERSION_MINOR}.${PS_VERSION_PATCH}")
else()
    set(PS_VERSION_DISPLAY "${PS_VERSION_MAJOR}.${PS_VERSION_MINOR}.${PS_VERSION_PATCH}-${PS_VERSION_PRERELEASE}")
endif()

# Windows VERSIONINFO — только числовой X.Y.Z.0 (предрелиз не входит в VERSIONINFO)
set(PS_VERSION_INFO "${PS_VERSION_MAJOR}.${PS_VERSION_MINOR}.${PS_VERSION_PATCH}.0")
set(PS_VERSION_TAG "v${PS_VERSION_DISPLAY}")
