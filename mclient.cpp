#include "mclient.h"

MClient::MClient(QObject *parent)
    : QObject(parent)
{
    m_clientThread = new QThread(this);
    m_clientThread->start();
    m_pConnectionClient = new ConnectionClient();
    m_pDataClient = new DataClient();
    m_pFrameManager = new FrameManager();

    // 2. Переносим его
    m_pConnectionClient->moveToThread(m_clientThread);
    m_pDataClient->moveToThread(m_clientThread);
    m_pFrameManager->moveToThread(m_clientThread);
    // 3. Запускаем поток
    m_clientThread->start();

    // DataClient signals
    connect(m_pDataClient,&DataClient::rdPacketReceived,this,&MClient::onRDPacketReceived);
    connect(m_pDataClient,&DataClient::fPacketReceived,m_pFrameManager,&FrameManager::onFPacketReceived);

    connect(m_pDataClient, &DataClient::addLog,[this](const QString& log){
        emit addLog(log);
    });
    connect(m_pDataClient, &DataClient::errorOccurred,[this](const QString& errorMessage){
        emit errorOccurred(errorMessage);
    });

    //ConnectionClient signals
    connect(m_pConnectionClient,&ConnectionClient::raPacketReceived,this,&MClient::onRAPacketReceived);

    connect(m_pConnectionClient,&ConnectionClient::connected,this,&MClient::onConnected);
    connect(m_pConnectionClient,&ConnectionClient::disconnected,this,&MClient::onDisconnected);

    connect(m_pConnectionClient, &ConnectionClient::addLog,[this](const QString& log){
        emit addLog(log);
    });
    connect(m_pConnectionClient, &ConnectionClient::errorOccurred,[this](const QString& errorMessage){
        emit errorOccurred(errorMessage);
    });

    //FrameManager signals
    connect(m_pFrameManager,&FrameManager::frameComplete,[this](uint64_t frameId, const QVideoFrame& frame){
        emit frameReceived(frameId,frame);
    });

    m_pFindTimer = new QTimer();
    connect(m_pFindTimer, &QTimer::timeout,this,&MClient::sendDPacket);
}

MClient::~MClient()
{
    m_clientThread->quit();
    m_clientThread->wait();

    m_pConnectionClient->deleteLater();
    m_pDataClient->deleteLater();
    m_pFrameManager->deleteLater();
}

MClient* MClient::instance(QObject *parent){
    static MClient* pInstance_(new MClient(parent));
    return pInstance_;
}

void MClient::setup(const QString &host, quint16 connectionPort, quint16 dataPort)
{
    QMetaObject::invokeMethod(m_pConnectionClient, "setup",Qt::QueuedConnection,
                              Q_ARG(QString, host),
                              Q_ARG(quint16, connectionPort));

    QMetaObject::invokeMethod(m_pDataClient, "setup",Qt::QueuedConnection,
                              Q_ARG(QString, host),
                              Q_ARG(uint16_t, dataPort));
}

void MClient::connectToServer()
{
    QMetaObject::invokeMethod(m_pDataClient, "connectToServer", Qt::QueuedConnection);
    QMetaObject::invokeMethod(m_pConnectionClient, "connectToServer", Qt::QueuedConnection);
    stopFindServer();
}

void MClient::disconnectFromServer()
{
    QMetaObject::invokeMethod(m_pConnectionClient, "disconnectFromServer", Qt::QueuedConnection);
    QMetaObject::invokeMethod(m_pDataClient, "disconnectFromServer", Qt::QueuedConnection);
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
    if (m_pDataClient->state() != QAbstractSocket::UnconnectedState){
        APacket pack;
        pack.type = 100;
        pack.width = AndroidTools::getDisplayWidth();
        pack.height = AndroidTools::getDisplayHeigth();
        pack.refreshRate = AndroidTools::getDisplayRefreshRate();
        pack.udpPort = m_pDataClient->localPort();
        sendDataC(pack.bytes());
        emit connected();
    }
}

void MClient::onDisconnected(){
    emit disconnected();
}

void MClient::onRAPacketReceived(const RAPacket& packet){
    if (packet.response == 0){
        emit authorized();
    }
}

void MClient::onRDPacketReceived(const RDPacket& packet){
    addLog(QString::number(packet.dataPort));
    addLog(QString::number(packet.connectionPort));
    addLog(QHostAddress(packet.ipAddress).toString());
    addLog(QString::number(packet.response));
    if (packet.response == 0){
        emit serverFound(QHostAddress(packet.ipAddress).toString(),packet.connectionPort,packet.dataPort);
    }
}

void MClient::startFindServer(){
    if (m_pFindTimer)
        m_pFindTimer->start(3000);

}

void MClient::stopFindServer(){
    if (m_pFindTimer)
        m_pFindTimer->stop();
}

void MClient::sendDataD(const QByteArray &data){
    QMetaObject::invokeMethod(m_pDataClient, "sendData", Qt::QueuedConnection,
                              Q_ARG(QByteArray, data));
}

void MClient::sendDataC(const QByteArray &data){
    QMetaObject::invokeMethod(m_pConnectionClient, "sendData", Qt::QueuedConnection,
                              Q_ARG(QByteArray, data));
}

void MClient::sendDPacket(){
    DPacket packet;
    QMetaObject::invokeMethod(m_pDataClient, "broadcastData", Qt::QueuedConnection,
                              Q_ARG(QByteArray, packet.bytes()));
}
