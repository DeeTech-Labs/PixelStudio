cmake_minimum_required(VERSION 3.16)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/PixelStudioVersion.cmake)

project(PixelStudio LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(PS_VERSION_FIX STREQUAL "")
    set(PS_VERSION_FIX_LITERAL "0")
else()
    set(PS_VERSION_FIX_LITERAL "'${PS_VERSION_FIX}'")
endif()

set(PS_GENERATED_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/include)
file(MAKE_DIRECTORY ${PS_GENERATED_INCLUDE_DIR}/app)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/app/AppVersion.h.in
    ${PS_GENERATED_INCLUDE_DIR}/app/AppVersion.h
    @ONLY
)

find_package(Qt6 REQUIRED COMPONENTS Quick QuickControls2 Gui Network Concurrent LinguistTools Svg)

qt_standard_project_setup(REQUIRES 6.8)

set(PIXELSTUDIO_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(PIXELSTUDIO_CORE_SOURCES
    src/processing/DisplayProfile.cpp
    src/processing/ImageFiltersPipeline.cpp
    src/processing/DisplayRasterizer.cpp
    src/processing/DisplayCodeGenerator.cpp
    src/processing/ConvertPipeline.cpp
    src/processing/ControllerCatalog.cpp
    src/processing/EncodingAnalyzer.cpp
    src/export/BatchExportService.cpp
    src/export/SpriteAtlasService.cpp
    src/persistence/AppPaths.cpp
    src/persistence/AppSettings.cpp
    src/persistence/SessionSettings.cpp
    src/persistence/ProjectService.cpp
    src/persistence/ProjectFormat.cpp
    src/persistence/StoredPath.cpp
    src/persistence/SettingsSchema.cpp
    src/io/ImageLoader.cpp
    src/io/BinaryExporter.cpp
    src/io/HeaderImportService.cpp
    src/io/WatchFolderService.cpp
)

add_library(pixelstudio_core STATIC ${PIXELSTUDIO_CORE_SOURCES})
target_include_directories(pixelstudio_core PUBLIC ${PIXELSTUDIO_SRC_DIR})
target_link_libraries(pixelstudio_core PUBLIC Qt6::Gui Qt6::Concurrent Qt6::Network)

set(APP_SOURCES
    src/main.cpp
    src/app/AppTranslations.cpp
    src/app/AppTranslations.h
    src/app/DisplayConverter.cpp
    src/app/DisplayConverter.h
    src/app/StudioTabController.cpp
    src/app/StudioTabController.h
    src/app/WinTaskbarRecent.cpp
    src/app/WinTaskbarRecent.h
)

qt_add_executable(appPixelStudio ${APP_SOURCES})

if(WIN32)
    target_sources(appPixelStudio PRIVATE resources/ico/PixelStudio.rc)
endif()

target_include_directories(appPixelStudio PRIVATE
    ${PIXELSTUDIO_SRC_DIR}
    ${PS_GENERATED_INCLUDE_DIR}
)

qt_add_translations(appPixelStudio
    SOURCE_TARGETS pixelstudio_core appPixelStudio
    TS_FILES
        i18n/pixelstudio_en.ts
        i18n/pixelstudio_ru.ts
    RESOURCE_PREFIX /i18n
)

set(PIXELSTUDIO_QML_FILES
    src/qml/I18n.qml
    src/qml/Main.qml
    src/qml/Theme.qml
    src/qml/controls/StudioField.qml
    src/qml/controls/StudioSection.qml
    src/qml/controls/StudioButton.qml
    src/qml/controls/StudioIcon.qml
    src/qml/controls/StudioNavIcon.qml
    src/qml/controls/StudioIconButton.qml
    src/qml/controls/StudioCheck.qml
    src/qml/controls/StudioSegmented.qml
    src/qml/controls/StudioSlider.qml
    src/qml/controls/StudioSpin.qml
    src/qml/controls/StudioCombo.qml
    src/qml/controls/StudioTextField.qml
    src/qml/controls/StudioScroll.qml
    src/qml/studio/StudioFrame.qml
    src/qml/studio/StudioViewport.qml
    src/qml/workspace/StudioDropCanvas.qml
    src/qml/workspace/StudioWorkspace.qml
    src/qml/workspace/StudioExportHub.qml
    src/qml/panels/InspectorDock.qml
    src/qml/panels/InspectorPageDisplay.qml
    src/qml/panels/InspectorPageImage.qml
    src/qml/panels/InspectorPageExport.qml
    src/qml/panels/ExportActionsPanel.qml
    src/qml/shell/CodeDock.qml
    src/qml/welcome/WelcomeScreen.qml
    src/qml/welcome/WelcomeBackdrop.qml
    src/qml/welcome/WelcomeDisplayViz.qml
    src/qml/welcome/WelcomeRecentCard.qml
    src/qml/shell/StudioTabBar.qml
    src/qml/shell/StudioTabChip.qml
)

# QML qsTr() defaults to file name as context; all studio QML use context "PixelStudio".
# All entries in pixelstudio_*.ts use context "PixelStudio" — align runtime lookup.
set_source_files_properties(${PIXELSTUDIO_QML_FILES}
    PROPERTIES QT_TR_CONTEXT "PixelStudio"
)
set_source_files_properties(src/qml/I18n.qml
    PROPERTIES QT_QML_SINGLETON_TYPE TRUE
)

file(GLOB PIXELSTUDIO_ICON_SVGS CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/icons/*.svg)
qt_add_resources(appPixelStudio "app_icons"
    PREFIX /icons
    BASE resources/icons
    FILES ${PIXELSTUDIO_ICON_SVGS}
)

qt_add_qml_module(appPixelStudio
    URI PixelStudio
    VERSION 1.0
    QML_FILES ${PIXELSTUDIO_QML_FILES}
    RESOURCE_PREFIX /
)

set_target_properties(appPixelStudio PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PS_VERSION_DISPLAY}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PS_VERSION_DISPLAY}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appPixelStudio
    PRIVATE pixelstudio_core Qt6::Quick Qt6::QuickControls2 Qt6::Gui Qt6::Network Qt6::Concurrent Qt6::Svg
)
if(WIN32)
    target_link_libraries(appPixelStudio PRIVATE dwmapi shell32 ole32 propsys shlwapi)
endif()

include(GNUInstallDirs)
install(TARGETS appPixelStudio
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
