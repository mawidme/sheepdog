# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html.
# For more examples on how to use CMake, see https://github.com/android/ndk-samples.

cmake_minimum_required(VERSION 3.22.1)

project("sheepdog")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall")

set(ANDROID_STL "c++_shared")

#set(QT_ANDROID_BASE_DIR "C:/Qt/qt6-android")
#set(QT_ANDROID_BASE_DIR "C:/Qt/qt6-android-arm64-v8a")
set(QT_ANDROID_BASE_DIR "C:/Qt/qt6-android-x86_64")

add_library(${CMAKE_PROJECT_NAME} SHARED
        native-lib.cpp
        ../../../../source/bark_wave.cpp
        ../../../../source/bonus.cpp
        ../../../../source/button_box.cpp
        ../../../../source/character.cpp
        ../../../../source/dog.cpp
        ../../../../source/editor_items.cpp
        ../../../../source/game_object.cpp
        ../../../../source/global_lists.cpp
        ../../../../source/globals.cpp
#        ../../../../source/highscore.cpp #TODO: port from flash
        ../../../../source/info_items.cpp
        ../../../../source/level_choose_items.cpp
        ../../../../source/level_element.cpp
        ../../../../source/level.cpp
        ../../../../source/main_game.cpp
        ../../../../source/main.cpp
        ../../../../source/mygraphicsscene.cpp
        ../../../../source/mygraphicsview.cpp
        ../../../../source/sheep.cpp
        ../../../../source/utils.cpp
        ../../../../source/xml.cpp
        )

#TODO: handle android events
add_library( app-glue
        STATIC
        ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c )

include_directories(../../../../source
        ${QT_ANDROID_BASE_DIR}/include
        )

add_library(QtCore SHARED IMPORTED)
set_target_properties(QtCore PROPERTIES IMPORTED_LOCATION ${QT_ANDROID_BASE_DIR}/lib/libQt6Core_${ANDROID_ABI}.so)

add_library(QtWidgets SHARED IMPORTED)
set_target_properties(QtWidgets PROPERTIES IMPORTED_LOCATION ${QT_ANDROID_BASE_DIR}/lib/libQt6Widgets_${ANDROID_ABI}.so)

add_library(QtSvgWidgets SHARED IMPORTED)
set_target_properties(QtSvgWidgets PROPERTIES IMPORTED_LOCATION ${QT_ANDROID_BASE_DIR}/lib/libQt6SvgWidgets_${ANDROID_ABI}.so)

add_library(QtGui SHARED IMPORTED)
set_target_properties(QtGui PROPERTIES IMPORTED_LOCATION ${QT_ANDROID_BASE_DIR}/lib/libQt6Gui_${ANDROID_ABI}.so)

#TODO
#add_library(QSound SHARED IMPORTED)
#set_target_properties(QSound IMPORTED_LOCATION ${QT_ANDROID_BASE_DIR}/lib/libQt6Core_${ANDROID_ABI}.so)

add_library(QtMultimedia SHARED IMPORTED)
set_target_properties(QtMultimedia PROPERTIES IMPORTED_LOCATION ${QT_ANDROID_BASE_DIR}/lib/libQt6Multimedia_${ANDROID_ABI}.so)

add_library(QtOpenGL SHARED IMPORTED)
set_target_properties(QtOpenGL PROPERTIES IMPORTED_LOCATION ${QT_ANDROID_BASE_DIR}/lib/libQt6OpenGL_${ANDROID_ABI}.so)

add_library(QtSvg SHARED IMPORTED)
set_target_properties(QtSvg PROPERTIES IMPORTED_LOCATION ${QT_ANDROID_BASE_DIR}/lib/libQt6Svg_${ANDROID_ABI}.so)

add_library(QtCore5Compat SHARED IMPORTED)
set_target_properties(QtCore5Compat PROPERTIES IMPORTED_LOCATION ${QT_ANDROID_BASE_DIR}/lib/libQt6Core5Compat_${ANDROID_ABI}.so)

add_library(Qt6Network SHARED IMPORTED)
set_target_properties(Qt6Network PROPERTIES IMPORTED_LOCATION ${QT_ANDROID_BASE_DIR}/lib/libQt6Network_${ANDROID_ABI}.so)

# Specifies libraries CMake should link to your target library. You
# can link libraries from various origins, such as libraries defined in this
# build script, prebuilt third-party libraries, or Android system libraries.
target_link_libraries(${CMAKE_PROJECT_NAME}
    android
    log
    Qt6Network
    QtCore
    QtCore5Compat
    QtGui
    QtMultimedia
    QtOpenGL
    QtSvg
    QtSvgWidgets
    QtWidgets
)