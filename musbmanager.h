#ifndef MUSBMANAGER_H
#define MUSBMANAGER_H

#include <QObject>
#include <QTimer>
#include <QTcpSocket>

#include "mappsettings.h"

class MUsbManager: public QObject
{
    Q_OBJECT

public:
    explicit MUsbManager(QObject *parent = nullptr);

public slots:
    void startFindServer();
    void stopFindServer();

signals:
    void serverFound(const QString &serverAddress, quint16 connectionPort, quint16 dataPort, Ms::ConnectionType connType);

private:
    void findServer();

    QTcpSocket* m_socket;

    QTimer* m_pConnectionTimer;

    QTimer* m_pFindTimer;
    bool m_finding = false;
};

#endif // MUSBMANAGER_H
