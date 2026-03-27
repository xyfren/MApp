QT += quick quickcontrols2 core network widgets multimedia

CONFIG += c++20

INCLUDEPATH += C:/libs/ffmpeg/android-armv8/include
LIBS += -LC:/libs/ffmpeg/android-armv8/lib
LIBS += -lavcodec -lavformat -lavutil -lswscale -lswresample

VERSION = 1.0.0

android {
    # Покрываем 95% устройств
    ANDROID_MIN_SDK_VERSION = 21

    # Целевая версия - последняя стабильная
    ANDROID_TARGET_SDK_VERSION = 33

    # Компилируем с последним SDK
    ANDROID_SDK_VERSION = 33

    # Поддерживаемые архитектуры
    ANDROID_ABIS = arm64-v8a
}
# CONFIG += link_pkgconfig
# PKGCONFIG += libavcodec libavformat libavutil libswscale libswresample

SOURCES += \
        TimeProfiler.cpp \
        ffmpegdecoder.cpp \
        framemanager.cpp \
        jpegdecoder.cpp \
        main.cpp \
        androidtools.cpp \
        connectionclient.cpp \
        dataclient.cpp \
        mclient.cpp \
        mplayer.cpp \
        musbmanager.cpp

resources.files = Main.qml MainPage.qml MonitorPage.qml SettingsPage.qml LogArea.qml ServerList.qml MPlayerOutput.qml MButton.qml mclient.cpp androidtools.h mclient.h dpacket.h
resources.prefix = /
RESOURCES += resources

# Additional import path used to resolve QML modules in Qt Creator's code model
# QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
# QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    TimeProfiler.h \
    dpacket.h \
    androidtools.h \
    apacket.h \
    connectionclient.h \
    dataclient.h \
    ffmpegdecoder.h \
    fpacket.h \
    framemanager.h \
    idecoder.h \
    jpegdecoder.h \
    mappsettings.h \
    mclient.h \
    mplayer.h \
    musbmanager.h \
    spacket.h

DISTFILES += \
    LogArea.qml \
    MButton.qml \
    MPlayerOutput.qml \
    MainPage.qml \
    MonitorPage.qml \
    ServerList.qml \
    SettingsPage.qml \
    android/AndroidManifest.xml \
    android/build.gradle \
    android/res/values/libs.xml \
    android/res/xml/qtprovider_paths.xml

contains(ANDROID_TARGET_ARCH,arm64-v8a) {
    ANDROID_PACKAGE_SOURCE_DIR = \
        $$PWD/android
}
