cmake_minimum_required(VERSION 3.16)

project(sheepdog VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.4 REQUIRED COMPONENTS Quick)

# qt_standard_project_setup()sig

qt_add_executable(sheepdog)
# qt_add_executable(projectname
#     MANUAL_FINALIZATION
#     ${PROJECT_SOURCES}
# )

set(PROJECT_SOURCES
    source/bark_wave.hpp
    source/bonus.cpp
    source/bonus.hpp
    source/bush.hpp
    source/button_box.cpp
    source/button_box.hpp
    source/character.cpp
    source/character.hpp
    source/deco.hpp
    source/dog.cpp
    source/dog.hpp
    source/editor_items.cpp
    source/editor_items.hpp
    source/exit.hpp
    source/fence.hpp
    source/game_object.cpp
    source/game_object.hpp
    source/global_lists.cpp
    source/global_lists.hpp
    source/globals.cpp
    source/globals.hpp
    source/info_items.cpp
    source/info_items.hpp
    source/level.cpp
    source/level.hpp
    source/level_choose_items.cpp
    source/level_choose_items.hpp
    source/level_element.cpp
    source/level_element.hpp
    source/level_types.hpp
    source/main.cpp
    source/main_game.cpp
    source/main_game.hpp
    source/mygraphicsscene.cpp
    source/mygraphicsscene.hpp
    source/mygraphicsview.cpp
    source/mygraphicsview.hpp
    source/sheep.cpp
    source/sheep.hpp
    source/tree.hpp
    source/types.hpp
    source/utils.cpp
    source/utils.hpp
    source/water.hpp
    source/xml.cpp
    source/xml.hpp
    ressources.qrc
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(sheepdog PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appsheepdog
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(sheepdog
    PRIVATE Qt6::Quick
)

include(GNUInstallDirs)
install(TARGETS sheepdog
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# qt_import_qml_plugins(sheepdog)
qt_finalize_executable(sheepdog)
