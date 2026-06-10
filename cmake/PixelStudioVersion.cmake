# Единый источник версии PixelStudio: Major.Minor.Patch[+fix]
# Пример с фиксом: PS_VERSION_FIX "b" → 1.2.3b
set(PS_VERSION_MAJOR 0)
set(PS_VERSION_MINOR 2)
set(PS_VERSION_PATCH 1)
set(PS_VERSION_FIX "")

if(PS_VERSION_FIX STREQUAL "")
    set(PS_VERSION_DISPLAY "${PS_VERSION_MAJOR}.${PS_VERSION_MINOR}.${PS_VERSION_PATCH}")
    set(PS_VERSION_FIX_CHAR "")
else()
    set(PS_VERSION_DISPLAY "${PS_VERSION_MAJOR}.${PS_VERSION_MINOR}.${PS_VERSION_PATCH}${PS_VERSION_FIX}")
    set(PS_VERSION_FIX_CHAR "${PS_VERSION_FIX}")
endif()

# Windows VERSIONINFO (четвёртый компонент — число; суффикс fix только в Display/теге)
set(PS_VERSION_INFO "${PS_VERSION_MAJOR}.${PS_VERSION_MINOR}.${PS_VERSION_PATCH}.0")
set(PS_VERSION_TAG "v${PS_VERSION_DISPLAY}")
