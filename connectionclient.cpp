#include "connectionclient.h"

ConnectionClient::ConnectionClient(QObject *parent)
    : QObject(parent)
    , m_socket(new QTcpSocket(this))
    , m_serverAddress("")
    , m_serverPort(0)
    , m_reconnectionTimer(new QTimer(this))
    , m_reconnectInterval(5000)
    // , m_manualDisconnect(false)
{

    m_socket->setSocketOption(QAbstractSocket::LowDelayOption, 1);

    m_reconnectionTimer->setSingleShot(true);

    connect(m_socket, &QTcpSocket::connected, this, &ConnectionClient::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &ConnectionClient::onDisconnected);
    connect(m_socket, &QTcpSocket::errorOccurred, this, &ConnectionClient::onErrorOccurred);
    connect(m_socket, &QTcpSocket::readyRead, this, &ConnectionClient::onReadyRead);
    connect(m_socket, &QTcpSocket::bytesWritten, this, &ConnectionClient::onBytesWritten);
    connect(m_socket, &QTcpSocket::stateChanged, this, &ConnectionClient::onStateChanged);

    // connect(m_reconnectionTimer, &QTimer::timeout, this, &ConnectionClient::reconnect);
    m_packetBuffer.reserve(sizeof(SPacket));
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
        disconnectFromServer();

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


// void ConnectionClient::setReconnectionTimeout(int ms)
// {
//     m_reconnectionTimer->setInterval(ms);
// }

// void ConnectionClient::reconnect()
// {
//     addLog("Переподключение");
//     if (!m_serverAddress.isNull() && m_serverPort > 0) {
//         connectToServer();
//     }
// }

void ConnectionClient::onConnected()
{
    // m_reconnectionTimer->stop();
    emit connected();
    addLog("Подключен к серверу");
}

void ConnectionClient::onDisconnected()
{
    cleanup();
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

// 1. В слоте чтения просто копируем всё в общий буфер
void ConnectionClient::onReadyRead()
{
    m_buffer.append(m_socket->readAll());
    processData(); // Теперь метод не принимает аргументов, а работает с m_buffer
}

// 2. Логика разбора буфера
void ConnectionClient::processData()
{
    // Работаем с данными по смещению m_readOffset
    while (m_buffer.size() >= (qsizetype)sizeof(uint16_t))
    {
        const uint16_t packetType = *reinterpret_cast<const uint16_t*>(m_buffer.constData());
        qsizetype totalSize = 0;

        // 2. Определяем СКОЛЬКО БАЙТ всего в этом пакете на сетевом уровне
        if (packetType == 101) {
            totalSize = sizeof(RAPacket);
        }
        else if (packetType == 201) {
            totalSize = sizeof(RDPacket);
        }
        else if (packetType == SPACKET_TYPE_H264 || packetType == SPACKET_TYPE_JPEG) {
            // Чтобы узнать размер SPacket, нужно прочитать заголовок (20 байт)
            if (m_buffer.size() < SPACKET_HEADER_SIZE) {
                return; // Ждем догрузки заголовка
            }

            // Читаем dataSize из буфера, не удаляя его
            // Поле dataSize находится на 18-м байте от начала структуры
            uint16_t dSize;
            std::memcpy(&dSize, m_buffer.constData() + 18, sizeof(uint16_t));
            // test
            // const SPacket* pPack = reinterpret_cast<const SPacket*>(m_buffer.constData());
            // if (pPack->partId == 0){
            //     TimeProfiler::instance().stamp("startFrame");
            // }
            // if (pPack->partId == pPack->totalParts - 1){
            //     TimeProfiler::instance().stamp("endFrame");
            // }

            //end test
            totalSize = SPACKET_HEADER_SIZE + dSize;

            // Защита от гигантских/кривых пакетов
            if (dSize > SPACKET_MAX_DATA_SIZE) {
                qDebug() << "Критическая ошибка: dataSize слишком большой!" << dSize;
                m_buffer.clear();
                return;
            }
        }
        else {
            qDebug() << "[Connection] Неизвестный тип:" << packetType << ". Сброс буфера.";
            m_buffer.clear();
            return;
        }

        // 3. Проверяем, пришел ли пакет целиком
        if (m_buffer.size() < totalSize) {
            return; // Ждем догрузки оставшихся байт
        }

        // 4. Пакет в буфере целиком. Извлекаем его ОДНИМ куском.
        QByteArray packetData = m_buffer.left(totalSize);
        m_buffer.remove(0, totalSize);

        // 5. Рассылаем сигналы
        if (packetType == 101) {
            emit raPacketReceived(RAPacket::fromBytes(packetData));
        }
        else if (packetType == 201) {
            emit rdPacketReceived(RDPacket::fromBytes(packetData));
        }
        else {
            // Для SPacket просто отправляем весь массив (Заголовок + Данные)
            emit sPacketReceived(packetData);
            // qDebug() << "[Connection] Пакет обработан. Тип:" << packetType << "Размер:" << totalSize;
        }
    }
}

void ConnectionClient::cleanup()
{
    m_buffer.clear();
    m_packetBuffer.clear();
    m_readOffset = 0;
}