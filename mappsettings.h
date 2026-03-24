#ifndef MAPPSETTINGS_H
#define MAPPSETTINGS_H

#include "qdebug.h"
#include <QObject>
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QtQmlIntegration/qqmlintegration.h>
#include <atomic>

#include "androidtools.h"

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
    Q_PROPERTY(int width READ getWidth WRITE setWidth)
    Q_PROPERTY(int height READ getHeight WRITE setHeight)
    Q_PROPERTY(int refreshRate READ getRefreshRate WRITE setRefreshRate)
    Q_PROPERTY(int quality READ getQuality WRITE setQuality)
    QML_ELEMENT
    QML_SINGLETON
public:

    static MAppSettings& getInstance() {
        static MAppSettings instance;
        return instance;
    }

    void load(){
        QSettings settings(getSettingsPath(),QSettings::IniFormat);

        // Load enum values as integers
        m_coderType = static_cast<Ms::CoderType>(
            settings.value("coderType", static_cast<int>(Ms::CoderType::Jpeg)).toInt()
            );

        // Load basic settings
        DisplayParameters param = AndroidTools::getDisplayParameters();

        m_width = settings.value("width", param.width).toInt();
        m_height = settings.value("height", param.height).toInt();
        m_refreshRate = settings.value("refreshRate", param.refreshRate).toInt();
        m_quality = settings.value("quality", 50).toInt();

        qDebug() << "Settings loaded from:" << settings.fileName();
        qDebug() << "CoderType:" << static_cast<int>(m_coderType.load());
        qDebug() << "ConnectionType:" << static_cast<int>(m_connectionType.load());
        qDebug() << "Width:" << m_width;
        qDebug() << "Height:" << m_height;
        qDebug() << "RefreshRate:" << m_refreshRate;
        qDebug() << "Quality:" << m_quality;

        // Emit signals to update QML
        // emit coderTypeChanged();
        // emit connectionTypeChanged();
        // emit widthChanged();
        // emit heightChanged();
        // emit refreshRateChanged();
        // emit qualityChanged();
    }

    void save(){
        QSettings settings(getSettingsPath(), QSettings::IniFormat);

        // Save enum values as integers
        settings.setValue("coderType", static_cast<int>(m_coderType.load()));

        // Save basic settings
        settings.setValue("width", m_width);
        settings.setValue("height", m_height);
        settings.setValue("refreshRate", m_refreshRate);
        settings.setValue("quality", m_quality);

        // Force sync to disk
        settings.sync();

        qDebug() << "Settings saved to:" << settings.fileName();
        qDebug() << "CoderType:" << static_cast<int>(m_coderType.load());
        qDebug() << "Width:" << m_width;
        qDebug() << "Height:" << m_height;
        qDebug() << "RefreshRate:" << m_refreshRate;
    }

    Ms::CoderType getCoderType() const {
        return m_coderType;
    }

    Ms::ConnectionType getConnectionType() const {
        return m_connectionType;
    }

    void setCoderType(Ms::CoderType aCoderType){
        m_coderType = aCoderType;
    }

    void setConnectionType(Ms::ConnectionType aConnectionType){
        m_connectionType = aConnectionType;
    }

    // Getters for new fields
    int getWidth() const {
        return m_width;
    }

    int getHeight() const {
        return m_height;
    }

    int getRefreshRate() const {
        return m_refreshRate;
    }

    int getQuality() const {
        return m_quality;
    }

    // Setters for new fields with notifications
    void setWidth(int width) {
        m_width = width;
    }

    void setHeight(int height) {
        m_height = height;
    }

    void setRefreshRate(int refreshRate) {
        m_refreshRate = refreshRate;
    }

    void setQuality(int quality){
        m_quality = quality;
    }

private:
    MAppSettings(){
        load();
    }
    ~MAppSettings(){
        save();
    }
    MAppSettings(const MAppSettings&) = delete;
    MAppSettings& operator=(const MAppSettings&) = delete;

    QString getSettingsPath() const {
        #ifdef Q_OS_ANDROID
                // On Android, use app-specific data directory
                QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
                QDir dir(dataPath);
                if (!dir.exists()) {
                    dir.mkpath(".");
                }
                return dataPath + "/app_settings.ini";
        #else
                // For desktop platforms, use application name
                return QCoreApplication::applicationName() + "_settings.ini";
        #endif
    }

    // Private fields
    int m_width = 0;
    int m_height = 0;
    int m_refreshRate = 0;
    int m_quality = 0;
    std::atomic<Ms::CoderType> m_coderType = Ms::CoderType::Null;
    std::atomic<Ms::ConnectionType> m_connectionType = Ms::ConnectionType::Null;
};

#endif // MAPPSETTINGS_H