#ifndef CONNECTIONCLIENT_H
#define CONNECTIONCLIENT_H

#include <QObject>
#include <QQuickItem>
#include <QString>
#include <QTcpSocket>
#include <QHostAddress>
#include <QTimer>
#include <QErrorMessage>
#include "apacket.h"

class ConnectionClient: public QObject
{
    Q_OBJECT
public:
    explicit ConnectionClient(QObject *parent = nullptr);
    ~ConnectionClient();

    bool isConnected() const;
    QAbstractSocket::SocketState state() const;
    qint16 localPort() const;

public slots:
    void setup(const QString &host, quint16 port);

    // Основные методы для работы с клиентом
    void connectToServer();
    void disconnectFromServer();

    void reconnect();

    void sendData(const QByteArray &data);
    void sendMessage(const QString &message);

    void setReconnectionTimeout(int ms = 5000);

signals:
    //внешние
    // Сигналы для отслеживания состояния
    void connected();
    void disconnected();
    // Сигналы приёма отправки
    void dataSent(qint64 bytes);

    void raPacketReceived(const RAPacket& packet);
    // Сигналы ошибок
    void stateChanged(QAbstractSocket::SocketState state);
    void errorOccurred(const QString &errorString);
    void addLog(const QString &log);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onBytesWritten(qint64 bytes);
    void onStateChanged(QAbstractSocket::SocketState state);
    void onErrorOccurred(QAbstractSocket::SocketError error);

private:
    void processData(const QByteArray &data);

    QTcpSocket* m_socket;

    QString m_serverAddress;
    uint16_t m_serverPort;

    QTimer *m_reconnectionTimer;
    int m_reconnectInterval;

    bool m_manualDisconnect;
};
#endif // CONNECTIONCLIENT_H
