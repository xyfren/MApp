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

    settings.setCoderType(Ms::CoderType::Jpeg);
    settings.setRefreshRate(30);

    AndroidTools* pAndroidTools = new AndroidTools(&engine);
    MClient* pMClient = MClient::instance(&engine);

    DisplayParameters param = AndroidTools::getDisplayParameters();
    qDebug() << param.width;
    qDebug() << param.height;
    qDebug() << param.refreshRate;

    qRegisterMetaType<Ms::ConnectionType>();
    qRegisterMetaType<Ms::CoderType>();

    qmlRegisterSingletonInstance("androidtools",1,0,"AndroidTools",pAndroidTools);
    qmlRegisterSingletonInstance("mapp",1,0,"MClient",pMClient);
    qmlRegisterSingletonInstance("mapp",1,0,"MAppSettings",&settings);
    qmlRegisterType<DisplayParameters>("mapp", 1, 0, "DisplayParameters");
    qmlRegisterType<MPlayer>("mapp",1,0,"MPlayer");
    qmlRegisterType<Ms>("mapp", 1, 0, "MsEnums");


    engine.load(QUrl("qrc:/Main.qml"));

    return app.exec();
}
