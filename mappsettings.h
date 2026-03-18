#ifndef MAPPSETTINGS_H
#define MAPPSETTINGS_H

#include "qdebug.h"
#include <QObject>
#include <QtQmlIntegration/qqmlintegration.h>
#include <atomic>

class Ms: public QObject {
    Q_OBJECT
public:
    enum class ConnectionType {
        Null,Usb,Wireless
    };
    Q_ENUM(ConnectionType);

    enum class CoderType {
        Null, FFmpeg, Jpeg
    };
    Q_ENUM(CoderType) // Use Q_ENUM_NS for namespaces
};


class MAppSettings: public QObject {
    Q_OBJECT
    Q_PROPERTY(Ms::CoderType coder READ getCoderType WRITE setCoderType)
    Q_PROPERTY(Ms::ConnectionType connection READ getConnectionType WRITE setConnectionType)
    QML_ELEMENT
    QML_SINGLETON
public:
    std::atomic<Ms::CoderType> coderType = Ms::CoderType::Null;
    std::atomic<Ms::ConnectionType> connectionType = Ms::ConnectionType::Null;
    static MAppSettings& getInstance() {
        static MAppSettings instance;
        return instance;
    }
    Ms::CoderType getCoderType() const {
        return coderType;
    }
    Ms::ConnectionType getConnectionType() const {
        return connectionType;
    }

    void setCoderType(Ms::CoderType aCoderType){
        coderType = aCoderType;
    }

    void setConnectionType(Ms::ConnectionType aConnectionType){
        connectionType = aConnectionType;
    }
private:
    MAppSettings() = default;
    ~MAppSettings() = default;
    MAppSettings(const MAppSettings&) = delete;
    MAppSettings& operator=(const MAppSettings&) = delete;
};

#endif // MAPPSETTINGS_H
