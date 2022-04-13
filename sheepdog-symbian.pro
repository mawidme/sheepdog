QT += core gui xml svg

HEADERS += \
    source/xml.hpp \
    source/water.hpp \
    source/utils.hpp \
    source/tree.hpp \
    source/sheep.hpp \
    source/mygraphicsview.hpp \
    source/mygraphicsscene.hpp \
    source/main_game.hpp \
    source/level_types.hpp \
    source/level_element.hpp \
    source/level_choose_items.hpp \
    source/level.hpp \
    source/info_items.hpp \
    source/globals.hpp \
    source/global_lists.hpp \
    source/game_object.hpp \
    source/fence.hpp \
    source/exit.hpp \
    source/editor_items.hpp \
    source/dog.hpp \
    source/deco.hpp \
    source/character.hpp \
    source/button_box.hpp \
    source/bush.hpp \
    source/bonus.hpp \
    source/bark_wave.hpp

SOURCES += \
    source/xml.cpp \
    source/utils.cpp \
    source/sheep.cpp \
    source/mygraphicsview.cpp \
    source/mygraphicsscene.cpp \
    source/main_game.cpp \
    source/main.cpp \
    source/level_element.cpp \
    source/level_choose_items.cpp \
    source/level.cpp \
    source/info_items.cpp \
    source/globals.cpp \
    source/global_lists.cpp \
    source/game_object.cpp \
    source/editor_items.cpp \
    source/dog.cpp \
    source/character.cpp \
    source/button_box.cpp \
    source/bonus.cpp \
    source/bark_wave.cpp

RESOURCES += \
    ressources.qrc

ICON = img/icon.svg
