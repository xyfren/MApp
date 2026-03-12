#ifndef DATACLIENT_H
#define DATACLIENT_H

#include <QObject>
#include <QAbstractSocket>
#include <QUdpSocket>
#include <QTimer>
#include <QErrorMessage>

#include "dpacket.h"
#include "spacket.h"

class DataClient : public QObject
{
    Q_OBJECT
public:
    explicit DataClient(QObject *parent = nullptr);
    ~DataClient();

    QAbstractSocket::SocketState state() const;
    qint16 localPort() const;

public slots:
    void setup(const QString& host, uint16_t  port);

    void connectToServer();
    void disconnectFromServer();

    void sendData(const QByteArray &data);
    void broadcastData(const QByteArray &data);

private slots:
    void onReadyRead();
    void onStateChanged(QAbstractSocket::SocketState state);
    void onErrorOccurred(QAbstractSocket::SocketError error);

signals:
    // External
    void errorOccurred(const QString &errorString);
    void addLog(const QString &log);
    // void dataReceived(const QByteArray data);
    void rdPacketReceived(const RDPacket &packet);
    void fPacketReceived(const QByteArray& data);
    void sPacketReceived(const QByteArray& data);

private:
    void processData(const QByteArray &data,const QHostAddress& senderAddress,quint16 senderPort);

    QUdpSocket* m_socket;
    QHostAddress m_serverAddress;
    uint16_t m_serverPort;
};

#endif // DATACLIENT_H
