#ifndef MAPPSETTINGS_H
#define MAPPSETTINGS_H

#include <atomic>

enum class ConnectionType {
    Null,Usb,Wireless
};

enum class CoderType {
    Null, FFmpeg, Jpeg
};

class MAppSettings {
public:
    std::atomic<CoderType> coderType = CoderType::Null;
    std::atomic<ConnectionType> connectionType = ConnectionType::Null;
    static MAppSettings& getInstance() {
        static MAppSettings instance;
        return instance;
    }
private:
    MAppSettings() = default;
    ~MAppSettings() = default;
    MAppSettings(const MAppSettings&) = delete;
    MAppSettings& operator=(const MAppSettings&) = delete;
};

#endif // MAPPSETTINGS_H
