cmake_minimum_required(VERSION 3.21)
project(cryptovisor VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Подсказки, где искать Qt, если он не в PATH (Homebrew на macOS, типовой путь на Windows).
if(APPLE AND EXISTS "/opt/homebrew/opt/qt")
    list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew/opt/qt")
endif()
if(WIN32 AND DEFINED ENV{QT_DIR})
    list(APPEND CMAKE_PREFIX_PATH "$ENV{QT_DIR}")
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Test)
qt_standard_project_setup()

# ── ядро: логика шифров без GUI ──
add_library(cryptocore STATIC
    src/core/caesar.cpp
    src/core/xor_cipher.cpp
    src/core/transposition.cpp
)
target_include_directories(cryptocore PUBLIC src)
target_link_libraries(cryptocore PUBLIC Qt6::Core)

set(UI_SOURCES
    src/ui/theme.cpp
    src/ui/draw.cpp
    src/ui/sidebar.cpp
    src/ui/tabbar.cpp
    src/ui/transport.cpp
    src/ui/main_window.cpp
    src/ui/scenes/scene.cpp
    src/ui/scenes/caesar_scene.cpp
    src/ui/scenes/xor_scene.cpp
    src/ui/scenes/transposition_scene.cpp
)
set(ASSET_FILES
    assets/fonts/Unbounded-Light.ttf
    assets/fonts/Unbounded-Regular.ttf
    assets/fonts/Unbounded-Medium.ttf
    assets/fonts/Unbounded-Bold.ttf
    assets/icon.png
)

# ── приложение ──
qt_add_executable(cryptovisor src/main.cpp ${UI_SOURCES})
qt_add_resources(cryptovisor "assets" PREFIX "/" BASE assets FILES ${ASSET_FILES})
target_link_libraries(cryptovisor PRIVATE cryptocore Qt6::Widgets)
set_target_properties(cryptovisor PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
    MACOSX_BUNDLE_BUNDLE_NAME "Криптовизор"
    MACOSX_BUNDLE_GUI_IDENTIFIER "dev.cryptovisor.app"
)
if(WIN32)
    # иконка exe
    file(WRITE "${CMAKE_BINARY_DIR}/app.rc" "IDI_ICON1 ICON \"${CMAKE_SOURCE_DIR}/assets/icon.ico\"\n")
    target_sources(cryptovisor PRIVATE "${CMAKE_BINARY_DIR}/app.rc")
endif()

# ── тесты ядра ──
enable_testing()
add_executable(test_ciphers tests/test_ciphers.cpp)
target_link_libraries(test_ciphers PRIVATE cryptocore)
add_test(NAME ciphers COMMAND test_ciphers)

# ── UI-библиотека для тестов и утилит ──
add_library(cryptoui STATIC ${UI_SOURCES})
target_link_libraries(cryptoui PUBLIC cryptocore Qt6::Widgets)
qt_add_resources(cryptoui "assets_ui" PREFIX "/" BASE assets FILES ${ASSET_FILES})

# скриншоты сцен (offscreen): cryptovisor_shots <папка>
qt_add_executable(cryptovisor_shots tests/screenshot_main.cpp)
target_link_libraries(cryptovisor_shots PRIVATE cryptoui)
set_target_properties(cryptovisor_shots PROPERTIES EXCLUDE_FROM_ALL ON)

# горячие клавиши и авто-режим
qt_add_executable(test_keys tests/keys_main.cpp)
target_link_libraries(test_keys PRIVATE cryptoui Qt6::Test)
add_test(NAME keys COMMAND test_keys)
set_tests_properties(keys PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
