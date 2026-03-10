QT += quick quickcontrols2 core network widgets multimedia

SOURCES += \
        framemanager.cpp \
        main.cpp \
        androidtools.cpp \
        connectionclient.cpp \
        dataclient.cpp \
        mclient.cpp \
        mplayer.cpp

resources.files = Main.qml MainPage.qml MonitorPage.qml LogArea.qml ServerList.qml MPlayerOutput.qml MButton.qml mclient.cpp mclient.h dpacket.h
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
    dpacket.h \
    androidtools.h \
    apacket.h \
    connectionclient.h \
    dataclient.h \
    fpacket.h \
    framemanager.h \
    mclient.h \
    mplayer.h

DISTFILES += \
    LogArea.qml \
    MButton.qml \
    MPlayerOutput.qml \
    MainPage.qml \
    MonitorPage.qml \
    ServerList.qml \
    android/AndroidManifest.xml \
    android/build.gradle \
    android/res/values/libs.xml \
    android/res/xml/qtprovider_paths.xml

contains(ANDROID_TARGET_ARCH,arm64-v8a) {
    ANDROID_PACKAGE_SOURCE_DIR = \
        $$PWD/android
}
