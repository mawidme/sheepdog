QT += core gui xml svg multimedia

OTHER_FILES += \
    android/AndroidManifest.xml \
    android/res/drawable-hdpi/icon.png \
    android/res/drawable-ldpi/icon.png \
    android/res/drawable-mdpi/icon.png \
    android/res/values/libs.xml \
    android/res/values/strings.xml \
    android/src/eu/licentia/necessitas/industrius/QtActivity.java \
    android/src/eu/licentia/necessitas/industrius/QtApplication.java \
    android/src/eu/licentia/necessitas/industrius/QtLayout.java \
    android/src/eu/licentia/necessitas/industrius/QtSurface.java \
    android/src/eu/licentia/necessitas/ministro/IMinistro.aidl \
    android/src/eu/licentia/necessitas/ministro/IMinistroCallback.aidl \
    android/src/eu/licentia/necessitas/mobile/QtAndroidContacts.java \
    android/src/eu/licentia/necessitas/mobile/QtCamera.java \
    android/src/eu/licentia/necessitas/mobile/QtFeedback.java \
    android/src/eu/licentia/necessitas/mobile/QtLocation.java \
    android/src/eu/licentia/necessitas/mobile/QtMediaPlayer.java \
    android/src/eu/licentia/necessitas/mobile/QtSensors.java \
    android/src/eu/licentia/necessitas/mobile/QtSystemInfo.java \
    android/res/drawable/icon.png \
    android/res/drawable/logo.png \
    android/res/layout/splash.xml \
    android/res/values/libs.xml \
    android/res/values/strings.xml \
    android/res/values-de/strings.xml \
    android/res/values-el/strings.xml \
    android/res/values-et/strings.xml \
    android/res/values-fa/strings.xml \
    android/res/values-fr/strings.xml \
    android/res/values-id/strings.xml \
    android/res/values-it/strings.xml \
    android/res/values-ja/strings.xml \
    android/res/values-ms/strings.xml \
    android/res/values-nb/strings.xml \
    android/res/values-nl/strings.xml \
    android/res/values-pl/strings.xml \
    android/res/values-pt-rBR/strings.xml \
    android/res/values-ro/strings.xml \
    android/res/values-rs/strings.xml \
    android/res/values-ru/strings.xml \
    android/res/values-zh-rCN/strings.xml \
    android/res/values-zh-rTW/strings.xml \
    android/src/org/kde/necessitas/ministro/IMinistro.aidl \
    android/src/org/kde/necessitas/ministro/IMinistroCallback.aidl \
    android/src/org/kde/necessitas/origo/QtActivity.java \
    android/src/org/kde/necessitas/origo/QtApplication.java

HEADERS += \
    source/fence.hpp \
    source/water.hpp \
    source/utils.hpp \
    source/tree.hpp \
    source/sheep.hpp \
    source/mygraphicsscene.hpp \
    source/main_game.hpp \
    source/level_element.hpp \
    source/level.hpp \
    source/globals.hpp \
    source/exit.hpp \
    source/dog.hpp \
    source/character.hpp \
    source/bush.hpp \
    source/bonus.hpp \
    source/global_lists.hpp \
    source/info_items.hpp \
    source/bark_wave.hpp \
    source/mygraphicsview.hpp \
    source/button_box.hpp \
    source/editor_items.hpp \
    source/game_object.hpp \
    source/xml.hpp \
    source/level_choose_items.hpp

SOURCES += \
    source/utils.cpp \
    source/sheep.cpp \
    source/mygraphicsscene.cpp \
    source/main_game.cpp \
    source/main.cpp \
    source/level_element.cpp \
    source/level.cpp \
    source/globals.cpp \
    source/dog.cpp \
    source/character.cpp \
    source/bonus.cpp \
    source/global_lists.cpp \
    source/info_items.cpp \
    source/bark_wave.cpp \
    source/mygraphicsview.cpp \
    source/button_box.cpp \
    source/editor_items.cpp \
    source/game_object.cpp \
    source/xml.cpp \
    source/level_choose_items.cpp

RESOURCES += \
    ressources.qrc

