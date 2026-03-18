#ifndef MCLIENT_H
#define MCLIENT_H

#include <QObject>
#include <QQuickItem>
#include <QString>
#include <QTcpSocket>
#include <QHostAddress>
#include <QTimer>
#include <QErrorMessage>

#include "mappsettings.h"
#include "androidtools.h"
#include "connectionclient.h"
#include "dataclient.h"
#include "framemanager.h"
#include "musbmanager.h"

#include "apacket.h"
#include "dpacket.h"
#include "spacket.h"

class MClient: public QObject
{
    Q_OBJECT
    QML_ELEMENT
public:

    ~MClient();

    static MClient* instance(QObject *parent = nullptr);

    void setUsbStatus(bool connected);

    Q_INVOKABLE void setup(const QString &host,quint16 connectionPort,quint16 dataPort);

    // Основные методы для работы с клиентом
    Q_INVOKABLE void connectToServer();
    Q_INVOKABLE void disconnectFromServer();
    Q_INVOKABLE void sendMessage(const QString &message);

    Q_INVOKABLE void startFindServer();
    Q_INVOKABLE void stopFindServer();

    Q_INVOKABLE bool isConnected() const;
    Q_INVOKABLE QAbstractSocket::SocketState state() const;

public slots:
    void onConnected();
    void onDisconnected();

    void onRAPacketReceived(const RAPacket& packet);
    void onRDPacketReceived(const RDPacket& packet);

signals:
    void addLog(const QString &log);
    void errorOccurred(const QString &errorString);

    void usbStatusChanged(bool connected);

    void serverFound(const QString &serverAddress,quint16 connectionPort,quint16 dataPort,Ms::ConnectionType connType);
    void frameReceived(const QVideoFrame& frame);

    void connected();
    void disconnected();

    void authorized();

private:
    explicit MClient(QObject *parent = nullptr);

    void sendDataD(const QByteArray &data);
    void sendDataC(const QByteArray &data);

    void sendAPacket();
    void sendDPacket();

    QThread* m_clientThread;

    ConnectionClient* m_pConnectionClient;
    DataClient* m_pDataClient;
    FrameManager* m_pFrameManager;
    MUsbManager* m_pMUsbManager;

    bool m_serverFinding = false;
};

#endif // MCLIENT_H
