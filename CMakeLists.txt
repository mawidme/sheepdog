cmake_minimum_required(VERSION 3.22)

project(sheepdog VERSION 0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(SHEEPDOG_LEGACY_QT "Build the legacy Qt version instead of the SDL one" OFF)

if(SHEEPDOG_LEGACY_QT)
    include(cmake/legacy_qt.cmake)
    return()
endif()

# --- SDL3 (git submodule, see external/SDL) --------------------------------
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
if(IOS)
    # HIDAPI pulls in CoreBluetooth, which needs a Bluetooth privacy string in
    # Info.plist; the game doesn't use gamepads, so leave it out.
    set(SDL_HIDAPI OFF CACHE BOOL "" FORCE)
endif()
add_subdirectory(external/SDL EXCLUDE_FROM_ALL)

if(MSVC)
    set(SHEEPDOG_WARNINGS /W4)
else()
    set(SHEEPDOG_WARNINGS -Wall -Wextra)
endif()

# --- core: platform independent utilities, no SDL ---------------------------
add_library(sheepdog_core STATIC
    src/core/random.cpp
    src/core/strings.cpp
)
target_include_directories(sheepdog_core PUBLIC src)
target_compile_options(sheepdog_core PRIVATE ${SHEEPDOG_WARNINGS})

# --- third party: cJSON (vendored, external/cjson/VERSION.txt) -----------------
add_library(cjson STATIC external/cjson/cJSON.c)
target_include_directories(cjson PUBLIC external/cjson)

# --- game: game logic and data, no SDL ---------------------------------------
add_library(sheepdog_game STATIC
    src/game/bonus.cpp
    src/game/character.cpp
    src/game/element.cpp
    src/game/level_data.cpp
    src/game/level_editor.cpp
    src/game/level_play.cpp
    src/game/world.cpp
)
target_link_libraries(sheepdog_game PUBLIC sheepdog_core PRIVATE cjson)
target_compile_options(sheepdog_game PRIVATE ${SHEEPDOG_WARNINGS})

# --- gfx: 2D rendering on top of SDL_Renderer -------------------------------
add_library(sheepdog_gfx STATIC
    src/gfx/font.cpp
    src/gfx/renderer.cpp
    src/gfx/tessellate.cpp
    src/gfx/texture.cpp
    src/gfx/texture_cache.cpp
    src/platform/assets.cpp
)
target_link_libraries(sheepdog_gfx PUBLIC sheepdog_core SDL3::SDL3 PRIVATE cjson)
target_compile_options(sheepdog_gfx PRIVATE ${SHEEPDOG_WARNINGS})

# --- audio: sound effects through SDL's audio stream --------------------------
add_library(sheepdog_audio STATIC
    src/audio/audio.cpp
    src/audio/mixer.cpp
)
target_link_libraries(sheepdog_audio PUBLIC sheepdog_gfx)
target_compile_options(sheepdog_audio PRIVATE ${SHEEPDOG_WARNINGS})

# --- game ------------------------------------------------------------------
set(SHEEPDOG_SOURCES
    src/main.cpp
    src/game_app.cpp
    src/ui/ui.cpp
    src/world_view.cpp
)
if(ANDROID)
    # SDLActivity loads the game from libmain.so
    add_library(sheepdog SHARED ${SHEEPDOG_SOURCES})
    set_target_properties(sheepdog PROPERTIES OUTPUT_NAME main)
else()
    add_executable(sheepdog ${SHEEPDOG_SOURCES})
endif()
target_link_libraries(sheepdog PRIVATE sheepdog_gfx sheepdog_game sheepdog_audio)
target_compile_options(sheepdog PRIVATE ${SHEEPDOG_WARNINGS})

# --- assets (assets/: baked images, font, levels, sounds) -------------------
# Android packages assets/ via Gradle (app/build.gradle.kts), the others here.
set(SHEEPDOG_ASSETS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/assets)
if(EMSCRIPTEN)
    target_link_options(sheepdog PRIVATE "--preload-file=${SHEEPDOG_ASSETS_DIR}@/assets")
    set_property(TARGET sheepdog APPEND PROPERTY LINK_DEPENDS ${SHEEPDOG_ASSETS_DIR})
elseif(IOS)
    file(GLOB_RECURSE SHEEPDOG_ASSET_FILES ${SHEEPDOG_ASSETS_DIR}/*)
    foreach(asset IN LISTS SHEEPDOG_ASSET_FILES)
        file(RELATIVE_PATH rel ${SHEEPDOG_ASSETS_DIR} ${asset})
        get_filename_component(rel_dir ${rel} DIRECTORY)
        set_source_files_properties(${asset} PROPERTIES MACOSX_PACKAGE_LOCATION "assets/${rel_dir}")
    endforeach()
    target_sources(sheepdog PRIVATE ${SHEEPDOG_ASSET_FILES})
elseif(NOT ANDROID)
    add_custom_command(TARGET sheepdog POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${SHEEPDOG_ASSETS_DIR} $<TARGET_FILE_DIR:sheepdog>/assets
        COMMENT "Copying assets")
endif()

# --- tests (desktop only) ----------------------------------------------------
if(NOT ANDROID AND NOT IOS AND NOT EMSCRIPTEN)
    enable_testing()
    add_executable(test_core tests/test_core.cpp)
    target_link_libraries(test_core PRIVATE sheepdog_core)
    target_compile_options(test_core PRIVATE ${SHEEPDOG_WARNINGS})
    add_test(NAME core COMMAND test_core)

    add_executable(test_gfx tests/test_gfx.cpp)
    target_link_libraries(test_gfx PRIVATE sheepdog_gfx)
    target_compile_options(test_gfx PRIVATE ${SHEEPDOG_WARNINGS})
    add_test(NAME gfx COMMAND test_gfx)

    add_executable(test_level tests/test_level.cpp)
    target_link_libraries(test_level PRIVATE sheepdog_game)
    target_compile_options(test_level PRIVATE ${SHEEPDOG_WARNINGS})
    target_compile_definitions(test_level PRIVATE SHEEPDOG_ASSETS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets")
    add_test(NAME level COMMAND test_level)

    add_executable(test_world tests/test_world.cpp)
    target_link_libraries(test_world PRIVATE sheepdog_game)
    target_compile_options(test_world PRIVATE ${SHEEPDOG_WARNINGS})
    target_compile_definitions(test_world PRIVATE SHEEPDOG_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
    add_test(NAME world COMMAND test_world)

    add_executable(test_level_play tests/test_level_play.cpp)
    target_link_libraries(test_level_play PRIVATE sheepdog_game)
    target_compile_options(test_level_play PRIVATE ${SHEEPDOG_WARNINGS})
    add_test(NAME level_play COMMAND test_level_play)

    add_executable(test_level_editor tests/test_level_editor.cpp)
    target_link_libraries(test_level_editor PRIVATE sheepdog_game)
    target_compile_options(test_level_editor PRIVATE ${SHEEPDOG_WARNINGS})
    add_test(NAME level_editor COMMAND test_level_editor)

    add_executable(test_mixer tests/test_mixer.cpp)
    target_link_libraries(test_mixer PRIVATE sheepdog_audio)
    target_compile_options(test_mixer PRIVATE ${SHEEPDOG_WARNINGS})
    add_test(NAME mixer COMMAND test_mixer)

    # whole app with the offscreen video driver and scripted input; needs the assets next to it
    add_executable(app_smoke tests/app_smoke.cpp src/game_app.cpp src/ui/ui.cpp src/world_view.cpp)
    target_include_directories(app_smoke PRIVATE src)
    target_link_libraries(app_smoke PRIVATE sheepdog_gfx sheepdog_game sheepdog_audio)
    target_compile_options(app_smoke PRIVATE ${SHEEPDOG_WARNINGS})
    add_custom_command(TARGET app_smoke POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${SHEEPDOG_ASSETS_DIR} $<TARGET_FILE_DIR:app_smoke>/assets)
    add_test(NAME app_smoke COMMAND app_smoke WORKING_DIRECTORY $<TARGET_FILE_DIR:app_smoke>)

    add_executable(test_ui tests/test_ui.cpp src/ui/ui.cpp)
    target_include_directories(test_ui PRIVATE src)
    target_link_libraries(test_ui PRIVATE sheepdog_gfx sheepdog_game)
    target_compile_options(test_ui PRIVATE ${SHEEPDOG_WARNINGS})
    target_compile_definitions(test_ui PRIVATE SHEEPDOG_ASSETS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets")
    add_test(NAME ui COMMAND test_ui)
endif()

if(EMSCRIPTEN)
    # emit sheepdog.html + .js + .wasm; serve the build dir over http to run it
    set_target_properties(sheepdog PROPERTIES SUFFIX ".html")
    target_link_options(sheepdog PRIVATE -sALLOW_MEMORY_GROWTH=1)
endif()

if(IOS)
    set(SHEEPDOG_BUNDLE_ID "ch.erglim.sheepdog" CACHE STRING "iOS bundle identifier (must be unique per Apple account)")
    set(SHEEPDOG_IOS_TEAM_ID "" CACHE STRING "Apple development team ID used for code signing")
    set_target_properties(sheepdog PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/cmake/ios/Info.plist.in
        MACOSX_BUNDLE_GUI_IDENTIFIER ${SHEEPDOG_BUNDLE_ID}
        MACOSX_BUNDLE_BUNDLE_NAME sheepdog
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
        XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER ${SHEEPDOG_BUNDLE_ID}
        XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2"   # iPhone + iPad
        XCODE_ATTRIBUTE_CODE_SIGN_STYLE Automatic
        XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "${SHEEPDOG_IOS_TEAM_ID}"
    )
endif()

if(WIN32)
    # SDL is linked statically; hide the console window in release builds
    set_target_properties(sheepdog PROPERTIES WIN32_EXECUTABLE $<CONFIG:Release>)
endif()
