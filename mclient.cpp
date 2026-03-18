#include "mclient.h"

MClient::MClient(QObject *parent)
    : QObject(parent)
{
    m_clientThread = new QThread(this);
    m_clientThread->start();
    m_pConnectionClient = new ConnectionClient();
    m_pDataClient = new DataClient();

    DisplayParameters param = AndroidTools::getDisplayParameters();
    m_pFrameManager = new FrameManager(param.width,param.height);

    m_pMUsbManager = new MUsbManager();
    // 2. Переносим его
    m_pConnectionClient->moveToThread(m_clientThread);
    m_pDataClient->moveToThread(m_clientThread);
    m_pFrameManager->moveToThread(m_clientThread);
    m_pMUsbManager->moveToThread(m_clientThread);
    // 3. Запускаем поток
    m_clientThread->start();

    connect(m_pMUsbManager,&MUsbManager::serverFound,[this](const QString &serverAddress, quint16 connectionPort, quint16 dataPort, Ms::ConnectionType connType){
        emit serverFound(serverAddress,connectionPort,dataPort,connType);
    });

    // DataClient signals
    connect(m_pDataClient,&DataClient::rdPacketReceived,this,&MClient::onRDPacketReceived);

    connect(m_pDataClient,&DataClient::sPacketReceived,m_pFrameManager,&FrameManager::onSPacketReceived);

    connect(m_pDataClient, &DataClient::addLog,[this](const QString& log){
        emit addLog(log);
    });
    connect(m_pDataClient, &DataClient::errorOccurred,[this](const QString& errorMessage){
        emit errorOccurred(errorMessage);
    });

    //ConnectionClient signals
    connect(m_pConnectionClient,&ConnectionClient::raPacketReceived,this,&MClient::onRAPacketReceived);
    connect(m_pConnectionClient,&ConnectionClient::sPacketReceived,m_pFrameManager,&FrameManager::onSPacketReceived);

    connect(m_pConnectionClient,&ConnectionClient::connected,this,&MClient::onConnected);
    connect(m_pConnectionClient,&ConnectionClient::disconnected,this,&MClient::onDisconnected);

    connect(m_pConnectionClient, &ConnectionClient::addLog,[this](const QString& log){
        emit addLog(log);
    });
    connect(m_pConnectionClient, &ConnectionClient::errorOccurred,[this](const QString& errorMessage){
        disconnectFromServer();
        emit errorOccurred(errorMessage);
    });

    //FrameManager signals
    connect(m_pFrameManager,&FrameManager::frameComplete,[this](const QVideoFrame& frame){
        emit frameReceived(frame);
    });
}

MClient::~MClient()
{
    m_clientThread->quit();
    m_clientThread->wait();

    m_pConnectionClient->deleteLater();
    m_pDataClient->deleteLater();
    m_pFrameManager->deleteLater();
    m_pMUsbManager->deleteLater();
}

MClient* MClient::instance(QObject *parent){
    static MClient* pInstance_(new MClient(parent));
    return pInstance_;
}

void MClient::setUsbStatus(bool connected) {
    // Просто испускаем сигнал. Qt сам разберется с очередью потоков.
    emit usbStatusChanged(connected);
}

void MClient::setup(const QString &host, quint16 connectionPort, quint16 dataPort)
{

    QMetaObject::invokeMethod(m_pConnectionClient, "setup",Qt::QueuedConnection,
                              Q_ARG(QString, host),
                              Q_ARG(quint16, connectionPort));
    if (MAppSettings::getInstance().connectionType == Ms::ConnectionType::Wireless){
        QMetaObject::invokeMethod(m_pDataClient, "setup",Qt::QueuedConnection,
                                  Q_ARG(QString, host),
                                  Q_ARG(uint16_t, dataPort));
    }
}

void MClient::connectToServer()
{
    stopFindServer();

    QMetaObject::invokeMethod(m_pConnectionClient, "connectToServer", Qt::QueuedConnection);
    if (MAppSettings::getInstance().connectionType == Ms::ConnectionType::Wireless){
        QMetaObject::invokeMethod(m_pDataClient, "connectToServer", Qt::QueuedConnection);
    }

}

void MClient::disconnectFromServer()
{
    QMetaObject::invokeMethod(m_pConnectionClient, "disconnectFromServer", Qt::QueuedConnection);
    if (MAppSettings::getInstance().connectionType == Ms::ConnectionType::Wireless){
        QMetaObject::invokeMethod(m_pDataClient, "disconnectFromServer", Qt::QueuedConnection);
    }
    startFindServer();
}


bool MClient::isConnected() const
{
    return m_pConnectionClient->isConnected();
}

QAbstractSocket::SocketState MClient::state() const
{
    return m_pConnectionClient->state();
}

void MClient::sendMessage(const QString &message){
    QByteArray bytes(message.toStdString().data());
    sendDataC(bytes);
}

void MClient::onConnected(){
    if (m_pConnectionClient->state() != QAbstractSocket::UnconnectedState){
        sendAPacket();
        emit connected();
    }
}

void MClient::onDisconnected(){
    QMetaObject::invokeMethod(m_pFrameManager, "cleanup", Qt::QueuedConnection);
    emit disconnected();
}

void MClient::onRAPacketReceived(const RAPacket& packet){
    if (packet.response == 0){
        emit authorized();
    }
}

void MClient::onRDPacketReceived(const RDPacket& packet){
    if (m_serverFinding){
        addLog(QString::number(packet.dataPort));
        addLog(QString::number(packet.connectionPort));
        addLog(QHostAddress(packet.ipAddress).toString());
        addLog(QString::number(packet.response));
        if (packet.response == 0){
            emit serverFound(QHostAddress(packet.ipAddress).toString(),packet.connectionPort,packet.dataPort,Ms::ConnectionType::Wireless);
        }
    }
}

void MClient::startFindServer(){
    QMetaObject::invokeMethod(m_pMUsbManager, "startFindServer", Qt::BlockingQueuedConnection);
    m_serverFinding = true;
}

void MClient::stopFindServer(){
    QMetaObject::invokeMethod(m_pMUsbManager, "stopFindServer", Qt::BlockingQueuedConnection);
    m_serverFinding = false;
}

void MClient::sendDataD(const QByteArray &data){
    QMetaObject::invokeMethod(m_pDataClient, "sendData", Qt::QueuedConnection,
                              Q_ARG(QByteArray, data));
}

void MClient::sendDataC(const QByteArray &data){
    QMetaObject::invokeMethod(m_pConnectionClient, "sendData", Qt::QueuedConnection,
                              Q_ARG(QByteArray, data));
}

void MClient::sendAPacket(){
    APacket pack;
    pack.type = 100;
    DisplayParameters param = AndroidTools::getDisplayParameters();
    pack.width = param.width;
    pack.height = param.height;
    pack.refreshRate = param.refreshRate;
    pack.udpPort = m_pDataClient->localPort();

    MAppSettings& settings = MAppSettings::getInstance();
    pack.coderType = settings.coderType;
    pack.connectionType = settings.connectionType;

    QMetaObject::invokeMethod(m_pFrameManager, "setDecoderType", Qt::QueuedConnection,
                              Q_ARG(Ms::CoderType, settings.coderType));

    sendDataC(pack.bytes());
}

void MClient::sendDPacket(){
    qDebug() << "Try connect";

}
