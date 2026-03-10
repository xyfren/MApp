#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "androidtools.h"
#include "mclient.h"
#include "mplayer.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QCoreApplication::setApplicationName("TestApp");
    QCoreApplication::setOrganizationName("xyfren");

    QQmlApplicationEngine engine;

    AndroidTools* pAndroidTools = new AndroidTools(&engine);
    MClient* pMClient = MClient::instance(&engine);

    qDebug() << AndroidTools::getDisplayHeigth();
    qDebug() << AndroidTools::getDisplayWidth();
    qDebug() << AndroidTools::getDisplayRefreshRate();

    qmlRegisterSingletonInstance("androidtools",1,0,"AndroidTools",pAndroidTools);
    qmlRegisterSingletonInstance("mclient",1,0,"MClient",pMClient);
    qmlRegisterType<MPlayer>("mplayer",1,0,"MPlayer");
    // qmlRegisterType<MClient>("mclient",1,0,"MClient");
    engine.load(QUrl("qrc:/Main.qml"));

    return app.exec();
}
