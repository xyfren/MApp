#include "dataclient.h"

DataClient::DataClient(QObject *parent):
    QObject(parent),
    m_socket(new QUdpSocket(this))
{
    m_socket->setReadBufferSize(0);
    connect(m_socket, &QUdpSocket::readyRead, this, &DataClient::onReadyRead);
    connect(m_socket, &QUdpSocket::stateChanged, this, &DataClient::onStateChanged);
    connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred),
            this, &DataClient::onErrorOccurred);

    if (!m_socket->bind(QHostAddress::Any, 12346, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint)) {
        emit errorOccurred("Failed to bind socket: " + m_socket->errorString());
        return;
    }
}

DataClient::~DataClient(){
    m_socket->close();
}

QAbstractSocket::SocketState DataClient::state() const{
    return m_socket->state();
}

qint16 DataClient::localPort() const{
    return m_socket->localPort();
}

void DataClient::setup(const QString& host, uint16_t  port){
    m_serverAddress = QHostAddress(host);
    m_serverPort = port;
}

void DataClient::connectToServer(){
    // Проверяем, если сокет уже привязан - сначала закрываем его
    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        m_socket->close();
        // Ждем, пока сокет полностью закроется (опционально)
    }

    if (m_serverAddress.isNull() || m_serverPort == 0) {
        emit errorOccurred("Invalid host address or port");
        return;
    }

    // Биндим сокет к любому доступному порту
    if (!m_socket->bind(QHostAddress::Any, 12346, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint)) {
        emit errorOccurred("Failed to bind socket: " + m_socket->errorString());
        return;
    }

    qDebug() << "Socket bound to port:" << m_socket->localPort();
    qDebug() << "Connected to server at:" << m_serverAddress.toString() << ":" << m_serverPort;
}

void DataClient::disconnectFromServer(){
    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        m_socket->close();
    }
}

void DataClient::sendData(const QByteArray &data){
    qint64 bytesSent = m_socket->writeDatagram(data, m_serverAddress, m_serverPort);
    qDebug() << "Bytes sent: " << bytesSent;
    if (bytesSent == -1) {
        emit errorOccurred("Failed to send data: " + m_socket->errorString());
    } else if (bytesSent != data.size()) {
        emit errorOccurred("Not all data was sent");
    }
}

void DataClient::broadcastData(const QByteArray &data){
    qint64 bytesSent = m_socket->writeDatagram(data, QHostAddress::Broadcast, 12346);
    qDebug() << "Bytes sent: " << bytesSent;
    addLog("бродкаст");
    if (bytesSent == -1) {
        emit errorOccurred("Failed to send data: " + m_socket->errorString());
    } else if (bytesSent != data.size()) {
        emit errorOccurred("Not all data was sent");
    }
}

void DataClient::onReadyRead(){
    while (m_socket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(m_socket->pendingDatagramSize());

        QHostAddress senderAddress;
        quint16 senderPort;

        qint64 bytesRead = m_socket->readDatagram(datagram.data(), datagram.size(),
                                                 &senderAddress, &senderPort);
        // qDebug() << "Bytes read: " << bytesRead;
        if (bytesRead == -1) {
            emit errorOccurred("Failed to read datagram: " + m_socket->errorString());
            continue;
        }
        processData(datagram,senderAddress,senderPort);
    }
}

void DataClient::processData(const QByteArray &data,const QHostAddress& senderAddress,quint16 senderPort){
    if (data.size() < 2) return ;

    const qint16 packetType = *(reinterpret_cast<const uint16_t*>(data.data()));
    qDebug() << "Новый пакет: " + QString::number(packetType);
    if (packetType == 300) {
        emit fPacketReceived(data);
    }
    else if (packetType == SPACKET_TYPE_H264 || packetType == SPACKET_TYPE_JPEG){
        if (senderAddress.toIPv4Address() == m_serverAddress.toIPv4Address() && senderPort == m_serverPort) {
            qDebug() << "Приш " ;
            emit sPacketReceived(data);
        }
    }
    else if (packetType == 201){
        RDPacket packet = RDPacket::fromBytes(data);
        emit rdPacketReceived(packet);
    }
    else {
        qDebug() << "Неизвестный пакет: " + QString::number(packetType);
    }
}

void DataClient::onStateChanged(QAbstractSocket::SocketState state)
{
    qDebug() << "Socket state changed:" << state;
}

void DataClient::onErrorOccurred(QAbstractSocket::SocketError error)
{
    QString errorString = m_socket->errorString();
    qDebug() << "Socket error:" << errorString;
    emit errorOccurred(errorString);
}
