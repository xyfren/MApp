#include "connectionclient.h"

ConnectionClient::ConnectionClient(QObject *parent)
    : QObject(parent)
    , m_socket(new QTcpSocket(this))
    , m_serverAddress("")
    , m_serverPort(0)
    , m_reconnectionTimer(new QTimer(this))
    , m_reconnectInterval(5000)
    , m_manualDisconnect(false)
{

    m_socket->setSocketOption(QAbstractSocket::LowDelayOption, 1);

    m_reconnectionTimer->setSingleShot(true);

    connect(m_socket, &QTcpSocket::connected, this, &ConnectionClient::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &ConnectionClient::onDisconnected);
    connect(m_socket, &QTcpSocket::errorOccurred, this, &ConnectionClient::onErrorOccurred);
    connect(m_socket, &QTcpSocket::readyRead, this, &ConnectionClient::onReadyRead);
    connect(m_socket, &QTcpSocket::bytesWritten, this, &ConnectionClient::onBytesWritten);
    connect(m_socket, &QTcpSocket::stateChanged, this, &ConnectionClient::onStateChanged);

    connect(m_reconnectionTimer, &QTimer::timeout, this, &ConnectionClient::reconnect);
}

ConnectionClient::~ConnectionClient()
{
    // m_manualDisconnect = true;
    m_socket->disconnectFromHost();
}

void ConnectionClient::setup(const QString &host, quint16 port)
{
    disconnectFromServer();

    m_serverAddress = host;
    m_serverPort = port;
}

void ConnectionClient::connectToServer()
{
    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        // Если уже подключены или подключаемся - сначала отключаем
        disconnectFromServer();

        // Ждем отключения через сигнал
        connect(m_socket, &QTcpSocket::disconnected, this, [this]() {
            disconnect(m_socket, &QTcpSocket::disconnected, this, nullptr);
            m_socket->connectToHost(m_serverAddress, m_serverPort);
        });
    }
    else{
        // m_manualDisconnect = false;
        m_socket->connectToHost(m_serverAddress, m_serverPort);
    }
    return;
}

void ConnectionClient::disconnectFromServer()
{
    // m_manualDisconnect = true;
    // m_reconnectionTimer->stop();

    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        m_socket->disconnectFromHost();
    }
}


bool ConnectionClient::isConnected() const
{
    return m_socket->state() == QAbstractSocket::ConnectedState;
}

QAbstractSocket::SocketState ConnectionClient::state() const
{
    return m_socket->state();
}

qint16 ConnectionClient::localPort() const{
    return m_socket->localPort();
}

void ConnectionClient::sendData(const QByteArray &data)
{
    if (!isConnected()) {
        m_socket->waitForConnected(500);
        if (!isConnected()) {
            emit errorOccurred("Не подключен к серверу");
        }
        return;
    }

    qint64 bytesWritten = m_socket->write(data);
    if (bytesWritten == -1) {
        emit errorOccurred("Ошибка отправки данных: " + m_socket->errorString());
    }
}

void ConnectionClient::sendMessage(const QString &message){
    QByteArray bytes(message.toStdString().data());
    sendData(bytes);
}


void ConnectionClient::setReconnectionTimeout(int ms)
{
    m_reconnectionTimer->setInterval(ms);
}

void ConnectionClient::reconnect()
{
    addLog("Переподключение");
    if (!m_serverAddress.isNull() && m_serverPort > 0) {
        connectToServer();
    }
}

void ConnectionClient::onConnected()
{
    // m_reconnectionTimer->stop();
    emit connected();
    addLog("Подключен к серверу");
}

void ConnectionClient::onDisconnected()
{
    emit disconnected();
    addLog("Отключен от сервера");

    // Авто-переподключение только если отключение не было ручным
    // if (!m_manualDisconnect && !m_serverAddress.isNull() && m_serverPort > 0) {
    //     m_reconnectionTimer->start();
    // }

    // m_manualDisconnect = false; // Сбрасываем флаг для следующих отключение
}

void ConnectionClient::onErrorOccurred(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error)

    QString errorString = m_socket->errorString();
    emit errorOccurred(errorString);

    // Авто-переподключение при ошибках (если не ручное отключение)
    if (!m_serverAddress.isNull() && m_serverPort > 0) {
        m_reconnectionTimer->start();
    }
}

void ConnectionClient::onReadyRead()
{
    QByteArray data = m_socket->readAll();

    processData(data);
}

void ConnectionClient::onBytesWritten(qint64 bytes)
{
    qDebug() << "Байты отправлены: " << bytes;
    addLog(QString("Байты отправлены: %1").arg(bytes));
    emit dataSent(bytes);
}

void ConnectionClient::onStateChanged(QAbstractSocket::SocketState state)
{
    emit stateChanged(state);
}

void ConnectionClient::processData(const QByteArray& data){
    qDebug() << "HAS NEW DATA";
    if (data.size() < 2) return;

    const qint16 packetType = *(reinterpret_cast<const uint16_t*>(data.data()));

    if (packetType == 101){
        RAPacket packet = RAPacket::fromBytes(data);
        emit raPacketReceived(packet);
    }
}
