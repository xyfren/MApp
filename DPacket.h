#ifndef DPACKET_H
#define DPACKET_H

#include <cstdint>
#include <QObject>
#include <QDebug>
#include <QByteArray>

#pragma pack(push, 1)
struct DPacket {
    quint16 type = 200;

    QByteArray bytes() const {
        QByteArray byteArray(sizeof(DPacket), 0);
        std::memcpy(byteArray.data(), this, sizeof(DPacket));
        return byteArray;
    }
};

struct RDPacket {
    quint16 type = 201;
    quint16 response = 0;
    quint32 ipAddress = 0;
    quint16 connectionPort = 0;
    quint16 dataPort = 0;

    QByteArray bytes() const {
        QByteArray byteArray(sizeof(RDPacket), 0);
        std::memcpy(byteArray.data(), this, sizeof(RDPacket));
        return byteArray;
    }

    static RDPacket fromBytes(const QByteArray& data) {
        RDPacket packet;

        if (data.size() < sizeof(RDPacket)) {
            qWarning() << "Not enough data to reconstruct APacket";

        }

        std::memcpy(&packet, data.data(), sizeof(RDPacket));
        return packet;
    }
};
#pragma pack(pop)


#endif // DPACKET_H
