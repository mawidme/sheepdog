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
add_subdirectory(external/SDL EXCLUDE_FROM_ALL)

# --- game ------------------------------------------------------------------
if(ANDROID)
    # SDLActivity loads the game from libmain.so
    add_library(sheepdog SHARED src/main.cpp)
    set_target_properties(sheepdog PROPERTIES OUTPUT_NAME main)
else()
    add_executable(sheepdog src/main.cpp)
endif()
target_link_libraries(sheepdog PRIVATE SDL3::SDL3)

if(MSVC)
    target_compile_options(sheepdog PRIVATE /W4)
else()
    target_compile_options(sheepdog PRIVATE -Wall -Wextra)
endif()

if(EMSCRIPTEN)
    # emit sheepdog.html + .js + .wasm; serve the build dir over http to run it
    set_target_properties(sheepdog PROPERTIES SUFFIX ".html")
    target_link_options(sheepdog PRIVATE -sALLOW_MEMORY_GROWTH=1)
endif()

if(WIN32)
    # SDL is linked statically; hide the console window in release builds
    set_target_properties(sheepdog PROPERTIES WIN32_EXECUTABLE $<CONFIG:Release>)
endif()
