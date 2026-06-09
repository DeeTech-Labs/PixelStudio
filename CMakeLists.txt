cmake_minimum_required(VERSION 3.16)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/PixelStudioVersion.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/CompileWarnings.cmake)

project(PixelStudio LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(PS_VERSION_FIX STREQUAL "")
    set(PS_VERSION_FIX_LITERAL "0")
else()
    set(PS_VERSION_FIX_LITERAL "'${PS_VERSION_FIX}'")
endif()

set(PS_GENERATED_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/include)
file(MAKE_DIRECTORY ${PS_GENERATED_INCLUDE_DIR}/app)
file(MAKE_DIRECTORY ${PS_GENERATED_INCLUDE_DIR}/translation)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/app/AppVersion.h.in
    ${PS_GENERATED_INCLUDE_DIR}/app/AppVersion.h
    @ONLY
)

file(GLOB PS_TRANSLATION_BUNDLES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/translations/??.json")
set(PS_TRANSLATION_CODE_ENTRIES "inline constexpr const char *kBundledLanguageCodes[] = {")
foreach(_ps_translation_bundle ${PS_TRANSLATION_BUNDLES})
    get_filename_component(_ps_translation_code ${_ps_translation_bundle} NAME_WE)
    string(APPEND PS_TRANSLATION_CODE_ENTRIES "\n    \"${_ps_translation_code}\",")
endforeach()
string(APPEND PS_TRANSLATION_CODE_ENTRIES "\n    nullptr\n};")
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/translation/TranslationStoreBuiltin.h.in
    ${PS_GENERATED_INCLUDE_DIR}/translation/TranslationStoreBuiltin.h
    @ONLY
)

find_package(Qt6 REQUIRED COMPONENTS Quick QuickControls2 Gui Network Concurrent Svg)

qt_standard_project_setup(REQUIRES 6.8)

set(PIXELSTUDIO_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(PIXELSTUDIO_CORE_SOURCES
    src/LogCategories.cpp
    src/processing/DisplayProfile.cpp
    src/processing/ImageFiltersPipeline.cpp
    src/processing/DisplayRasterizer.cpp
    src/processing/DisplayCodeGenerator.cpp
    src/processing/ConvertPipeline.cpp
    src/processing/EncodingAnalyzer.cpp
    src/processing/PixelFormatCatalog.cpp
    src/processing/PixelFormatPacking.cpp
    src/processing/PreviewPalette.cpp
    src/export/BatchExportService.cpp
    src/export/SpriteAtlasService.cpp
    src/persistence/AppPaths.cpp
    src/persistence/AppSettings.cpp
    src/persistence/SessionSettings.cpp
    src/persistence/ProjectService.cpp
    src/persistence/ProjectFormat.cpp
    src/persistence/ProjectFormatIo.cpp
    src/persistence/ProjectFormatEncoder.cpp
    src/persistence/ProjectFormatDecoder.cpp
    src/persistence/RecentPreview.cpp
    src/persistence/StoredPath.cpp
    src/persistence/SettingsSchema.cpp
    src/app/code/CodeSyntaxTheme.cpp
    src/io/ImageLoader.cpp
    src/io/BinaryExporter.cpp
    src/io/HeaderImportService.cpp
    src/io/WatchFolderService.cpp
    src/translation/TranslationStore.cpp
    src/translation/JsonTranslator.cpp
)

add_library(pixelstudio_core STATIC ${PIXELSTUDIO_CORE_SOURCES})
target_include_directories(pixelstudio_core PUBLIC
    ${PIXELSTUDIO_SRC_DIR}
    ${PS_GENERATED_INCLUDE_DIR}
)
target_link_libraries(pixelstudio_core PUBLIC Qt6::Gui Qt6::Concurrent Qt6::Network)
pixelstudio_apply_warnings(pixelstudio_core)

set(APP_SOURCES
    src/main.cpp
    src/app/shell/AppTranslations.cpp
    src/app/shell/AppTranslations.h
    src/app/code/CodeSyntaxHighlighter.cpp
    src/app/code/CodeSyntaxHighlighter.h
    src/app/studio/DisplayConverter.cpp
    src/app/studio/DisplayConverter.h
    src/app/studio/SessionPersistenceService.cpp
    src/app/studio/SessionPersistenceService.h
    src/app/preview/PreviewImageProvider.cpp
    src/app/preview/PreviewImageProvider.h
    src/app/studio/controllers/image/ImageFilterController.cpp
    src/app/studio/controllers/image/ImageTransformController.cpp
    src/app/studio/controllers/output/DisplayOutputController.cpp
    src/app/studio/controllers/output/CodeGenController.cpp
    src/app/studio/controllers/output/ViewportController.cpp
    src/app/studio/pipeline/ImagePipelineController.cpp
    src/app/studio/controllers/project/ProjectController.cpp
    src/app/studio/controllers/export/ExportController.cpp
    src/app/tabs/StudioTabController.cpp
    src/app/tabs/StudioTabController.h
    src/app/tabs/TabPersistence.cpp
    src/app/tabs/TabSessionStore.cpp
    src/app/tabs/TabProjectSync.cpp
    src/app/tabs/TabRestoreService.cpp
    src/app/tabs/TabPreloadService.cpp
    src/app/tabs/TabWelcomeService.cpp
    src/app/studio/TabStateService.cpp
    src/app/shell/WinTaskbarRecent.cpp
    src/app/shell/WinTaskbarRecent.h
    src/app/shell/StudioViewMode.cpp
    src/app/shell/WorkspaceContext.cpp
    src/app/shell/WorkspaceContext.h
    src/app/panels/InspectorImagePresenter.cpp
    src/app/panels/InspectorImagePresenter.h
    src/app/panels/InspectorDisplayPresenter.cpp
    src/app/panels/InspectorDisplayPresenter.h
)

qt_add_executable(PixelStudio ${APP_SOURCES})

if(WIN32)
    target_sources(PixelStudio PRIVATE resources/ico/PixelStudio.rc)
endif()

target_include_directories(PixelStudio PRIVATE
    ${PIXELSTUDIO_SRC_DIR}
    ${PS_GENERATED_INCLUDE_DIR}
)

set(PIXELSTUDIO_QML_FILES
    src/qml/Main.qml
    src/qml/Theme.qml
    src/qml/welcome/WelcomeScreen.qml
    src/qml/welcome/WelcomeBackdrop.qml
    src/qml/welcome/WelcomeRecentCard.qml
    src/qml/welcome/WelcomeFooterNav.qml
    src/qml/welcome/WelcomeBrandMark.qml
    src/qml/welcome/WelcomeSparkle.qml
    src/qml/panels/InspectorDock.qml
    src/qml/panels/InspectorTabRow.qml
    src/qml/panels/InspectorPageDisplay.qml
    src/qml/panels/InspectorPageImage.qml
    src/qml/panels/inspector/InspectorImageInfo.qml
    src/qml/panels/inspector/InspectorImageAdjustments.qml
    src/qml/panels/inspector/InspectorImageAdvanced.qml
    src/qml/panels/inspector/InspectorImageTransform.qml
    src/qml/panels/inspector/InspectorImageTone.qml
    src/qml/panels/inspector/InspectorImageAdvancedColor.qml
    src/qml/panels/inspector/InspectorImageDithering.qml
    src/qml/panels/inspector/InspectorImageEdgesMask.qml
    src/qml/panels/ExportActionsPanel.qml
    src/qml/shell/WorkflowRouter.qml
    src/qml/shell/StudioCommands.qml
    src/qml/shell/StudioMenuFromCommands.qml
    src/qml/shell/MainMenuBar.qml
    src/qml/shell/MainDialogs.qml
    src/qml/shell/dialogs/ProjectDialogs.qml
    src/qml/shell/dialogs/ExportDialogs.qml
    src/qml/shell/dialogs/SettingsDialogs.qml
    src/qml/features/export/ExportHub.qml
    src/qml/controls/InspectorSection.qml
    src/qml/shell/ExitConfirmDialog.qml
    src/qml/settings/SettingsPage.qml
    src/qml/settings/SettingsNavButton.qml
    src/qml/settings/SyntaxColorsEditor.qml
    src/qml/shell/studio/StudioShell.qml
    src/qml/shell/studio/StudioTabBar.qml
    src/qml/shell/studio/StudioTabChip.qml
    src/qml/shell/studio/StudioToolbar.qml
    src/qml/shell/studio/StudioStatusBar.qml
    src/qml/shell/studio/StudioToast.qml
    src/qml/shell/studio/StudioDialog.qml
    src/qml/shell/studio/StudioConfirmFooter.qml
    src/qml/shell/studio/StudioOkFooter.qml
    src/qml/dialogs/AboutDialog.qml
    src/qml/dialogs/AboutBrandMark.qml
    src/qml/workspace/StudioDropCanvas.qml
    src/qml/workspace/StudioLayout.qml
    src/qml/workspace/CodePanel.qml
    src/qml/workspace/CodeSyntaxView.qml
    src/qml/workspace/dock/LeftAssetDock.qml
    src/qml/workspace/dock/PalettePanel.qml
    src/qml/workspace/dock/ProjectPanel.qml
    src/qml/studio/StudioViewport.qml
    src/qml/studio/PixelPanel.qml
    src/qml/studio/PixelFrame.qml
    src/qml/studio/PixelTab.qml
    src/qml/studio/PixelDottedRule.qml
    src/qml/studio/StudioBackdrop.qml
    src/qml/controls/StudioField.qml
    src/qml/controls/StudioSection.qml
    src/qml/controls/StudioButton.qml
    src/qml/controls/StudioIcon.qml
    src/qml/controls/StudioIconButton.qml
    src/qml/controls/StudioCheck.qml
    src/qml/controls/StudioSegmented.qml
    src/qml/controls/StudioSlider.qml
    src/qml/controls/StudioSpin.qml
    src/qml/controls/StudioCombo.qml
    src/qml/controls/StudioBoundCombo.qml
    src/qml/controls/StudioTextField.qml
    src/qml/controls/StudioScroll.qml
)

qt_add_resources(PixelStudio "app_translations"
    PREFIX /translations
    BASE translations
    FILES ${PS_TRANSLATION_BUNDLES}
)
set_source_files_properties(src/qml/shell/WorkflowRouter.qml
    PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
set_source_files_properties(src/qml/shell/StudioCommands.qml
    PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

file(GLOB PIXELSTUDIO_ICON_PNGS CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/icons/*.png)
qt_add_resources(PixelStudio "app_icons"
    PREFIX /icons
    BASE resources/icons
    FILES ${PIXELSTUDIO_ICON_PNGS}
)

qt_add_resources(PixelStudio "app_welcome_assets"
    PREFIX /
    BASE resources
    FILES
        resources/fonts/PressStart2P-Regular.ttf
        resources/ico/PixelStudio.ico
        resources/ico/PixelStudio.png
)

qt_add_qml_module(PixelStudio
    URI PixelStudio
    VERSION 1.0
    QML_FILES ${PIXELSTUDIO_QML_FILES}
    RESOURCE_PREFIX /
)

set_target_properties(PixelStudio PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PS_VERSION_DISPLAY}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PS_VERSION_DISPLAY}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(PixelStudio
    PRIVATE pixelstudio_core Qt6::Quick Qt6::QuickControls2 Qt6::Gui Qt6::Network Qt6::Concurrent Qt6::Svg
)
if(WIN32)
    target_link_libraries(PixelStudio PRIVATE dwmapi shell32 ole32 propsys shlwapi)
endif()

pixelstudio_apply_warnings(PixelStudio)

include(GNUInstallDirs)
install(TARGETS PixelStudio
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
