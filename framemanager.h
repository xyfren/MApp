#ifndef FRAMEMANAGER_H
#define FRAMEMANAGER_H

#include <QObject>
#include <QByteArray>
#include <fpacket.h>
#include <QImage>
#include <QVideoFrame>

struct FrameBuffer {
    QImage image;
    QVideoFrame frame;
    QByteArray data;
    uint64_t currentFrameId = 0xFFFFFFFFFFFFFFFF;
    uint16_t partsReceived = 0;
    uint16_t totalParts = 0;
};

class FrameManager : public QObject
{
    Q_OBJECT
public:
    explicit FrameManager(QObject *parent = nullptr);

public slots:
    void onFPacketReceived(const QByteArray data);

signals:
    // void frameComplete(uint32_t frameId, const QByteArray& frameData);
    void frameComplete(uint32_t frameId, const QVideoFrame& frameData);

private:
    // Индекс 0 для четных, 1 для нечетных frameId
    uint64_t m_lastFrame = 0;
    FrameBuffer m_buffers[2];
};

#endif // FRAMEMANAGER_H
