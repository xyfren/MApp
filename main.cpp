#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "androidtools.h"
#include "mclient.h"
#include "mplayer.h"
#include <QDebug>

extern "C" {
#include <libavcodec/avcodec.h>
}

#include <iostream>
#include <iomanip>

void printAvailableEncoders() {
    void* i = nullptr;
    const AVCodec* codec = nullptr;

    qDebug() << "=== Доступные видео-энкодеры в вашей системе ===";
    qDebug() << "Имя " << "Описание";
    qDebug() << "------------------------------------------------";

    // Проходим по всем зарегистрированным кодекам
    while ((codec = av_codec_iterate(&i))) {
        // Нас интересуют только декодеры и только видео
        if (av_codec_is_decoder(codec) && codec->type == AVMEDIA_TYPE_VIDEO) {
            qDebug() << codec->name << " "
                     << codec->long_name;
        }
    }
    qDebug() << "================================================";
}


int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QCoreApplication::setApplicationName("TestApp");
    QCoreApplication::setOrganizationName("xyfren");

    QQmlApplicationEngine engine;

    MAppSettings& settings = MAppSettings::getInstance();
    settings.coderType = CoderType::Jpeg;
    settings.connectionType = ConnectionType::Wireless;

    AndroidTools* pAndroidTools = new AndroidTools(&engine);
    MClient* pMClient = MClient::instance(&engine);

    DisplayParameters param = AndroidTools::getDisplayParameters();
    qDebug() << param.width;
    qDebug() << param.height;
    qDebug() << param.refreshRate;

    // printAvailableEncoders();

    qmlRegisterSingletonInstance("androidtools",1,0,"AndroidTools",pAndroidTools);
    qmlRegisterSingletonInstance("mclient",1,0,"MClient",pMClient);
    qmlRegisterType<MPlayer>("mplayer",1,0,"MPlayer");
    engine.load(QUrl("qrc:/Main.qml"));

    return app.exec();
}
