# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html.
# For more examples on how to use CMake, see https://github.com/android/ndk-samples.

cmake_minimum_required(VERSION 3.22.1)

project("sheepdog")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall")

add_library(${CMAKE_PROJECT_NAME} SHARED
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

#TODO
#target_link_libraries( # Specifies the target library.
#                       libqt...so)

#TODO
#add_library( app-glue
#        STATIC
#        ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c )

include_directories(../../../../source
#        qt-everywhere-src-5.15.2/qtbase/include
#        qt-everywhere-src-5.15.2/qtmultimedia/include
        C:/Qt/qt6-android/include
        )

# Specifies libraries CMake should link to your target library. You
# can link libraries from various origins, such as libraries defined in this
# build script, prebuilt third-party libraries, or Android system libraries.
target_link_libraries(${CMAKE_PROJECT_NAME}
        # List libraries link to the target library
        android
        log)