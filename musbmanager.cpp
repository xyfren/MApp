#include "musbmanager.h"

#include "mclient.h"

// #include <QJniObject>
// #include <QJniEnvironment>

// extern "C" JNIEXPORT void JNICALL
// Java_org_qtproject_example_UsbReceiver_onUsbStateChanged(JNIEnv *env, jobject thiz, jboolean isConnected) {

//     // Получаем доступ к нашему объекту MClient
//     MClient* client = MClient::instance();

//     if (client) {
//         MClient* client = MClient::instance();
//         if (client) {
//             client->setUsbStatus(isConnected);
//         }
//     }
// }


MUsbManager::MUsbManager(QObject *parent)
    : QObject(parent)
{
    m_socket = new QTcpSocket(this);
    m_pFindTimer = new QTimer(this);
    QObject::connect(m_pFindTimer, &QTimer::timeout, this, &MUsbManager::findServer);

    m_pConnectionTimer = new QTimer(this);

    QObject::connect(m_socket, &QTcpSocket::connected, [this]() {

        // Останавливаем таймер таймаута
        m_pConnectionTimer->stop();

        // Важно: не закрываем сразу, ждем ответ или закрываем через таймер
        QTimer::singleShot(2000, m_socket, [this]() {
            if (m_socket->isOpen()) {
                qDebug() << "📤 Закрываем соединение после проверки";
                m_socket->disconnectFromHost();
                m_finding = false;
            }
        });
    });

    QObject::connect(m_socket, &QTcpSocket::readyRead, [this]() {
        qDebug() << "fdsf";
        QByteArray data = m_socket->readAll();
        qDebug() << data.size();

        if (data.size() < 2) return;

        const quint16 packetType = *(reinterpret_cast<const uint16_t*>(data.data()));
        qDebug() << packetType;
        if (packetType == 201){
            RDPacket packet = RDPacket::fromBytes(data);
            qDebug() << QString::number(packet.dataPort);
            qDebug() << QString::number(packet.connectionPort);
            qDebug() << QHostAddress(packet.ipAddress).toString();
            qDebug() << QString::number(packet.response);
            if (packet.response == 0){
                qDebug() << "Сервер доступен";
                emit serverFound("usb",packet.connectionPort,packet.dataPort,Ms::ConnectionType::Usb);
            }
        }
    });


    // При ошибке
    QObject::connect(m_socket, &QTcpSocket::errorOccurred, [this](QTcpSocket::SocketError error) {
        qDebug() << "❌ Ошибка подключения к ADB серверу:" << m_socket->errorString();
        m_pConnectionTimer->stop();
        m_socket->disconnectFromHost(); // Инициируем отключение
        m_finding = false;
    });

    // При отключении (важно для очистки памяти!)
    QObject::connect(m_socket, &QTcpSocket::disconnected, [this]() {
        qDebug() << "🔌 Соединение закрыто, очищаем ресурсы";
        m_finding = false;
    });

    // Таймаут подключения
    QObject::connect(m_pConnectionTimer, &QTimer::timeout, [this]() {
        qDebug() << "⏱️ Таймаут подключения к ADB серверу";
        if (m_socket->state() == QTcpSocket::ConnectingState ||
            m_socket->state() == QTcpSocket::HostLookupState) {
            m_socket->abort(); // Немедленно прерываем попытку подключения
        } else {
            m_socket->disconnectFromHost();
        }
        m_finding = false;
    });
}

void MUsbManager::startFindServer(){
    m_pFindTimer->start(3000);
}

void MUsbManager::stopFindServer(){
    m_pFindTimer->stop();
    m_socket->abort();
}

void MUsbManager::findServer(){
    if (!m_finding){
        m_finding = true;

        m_pConnectionTimer->setSingleShot(true);
        m_pConnectionTimer->start(1000);

        qDebug() << "🔄 Проверка ADB сервера на localhost:12345...";
        m_socket->connectToHost("127.0.0.1", 12345);
    }
}
