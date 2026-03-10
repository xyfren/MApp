#ifndef APACKET_H
#define APACKET_H

#include <QObject>
#include <QByteArray>
#include <QDebug>

#pragma pack(push, 1)
struct APacket {
    quint16 type = 100;
    quint16 width = 0;
    quint16 height = 0;
    quint16 refreshRate = 0;
    quint16 udpPort = 0;

    QByteArray bytes() const {
        QByteArray byteArray(sizeof(APacket), 0);
        std::memcpy(byteArray.data(), this, sizeof(APacket));
        return byteArray;
    }
};

struct RAPacket {
    quint16 type = 101;
    quint16 response = 0;

    QByteArray bytes() const {
        QByteArray byteArray(sizeof(RAPacket), 0);
        std::memcpy(byteArray.data(), this, sizeof(RAPacket));
        return byteArray;
    }

    static RAPacket fromBytes(const QByteArray& data) {
        RAPacket packet;

        if (static_cast<unsigned long>(data.size()) < sizeof(RAPacket)) {
            qDebug() << "Not enough data to reconstruct RAPacket";
        }

        std::memcpy(&packet, data.data(), sizeof(RAPacket));
        return packet;
    }
};
#pragma pack(pop)

#endif // APACKET_H
