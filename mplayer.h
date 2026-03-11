#ifndef MPLAYER_H
#define MPLAYER_H

#include <QObject>
#include <QQuickItem>
#include <QString>
#include <QTcpSocket>
#include <QHostAddress>
#include <QTimer>
#include <QErrorMessage>
#include <QVideoSink>
#include <QVideoFrame>
#include <QImage>
#include "mclient.h"

class MPlayer: public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QObject* videoOutput READ videoOutput WRITE setVideoOutput)
public:
    explicit MPlayer(QObject* parent = nullptr);
    ~MPlayer();

    QObject* videoOutput() const;
    void setVideoOutput(QObject* videoOutput);

private slots:
    // void onFrameReceived(uint64_t frameId, const QByteArray& frameData);
    void onFrameReceived(const QVideoFrame& frameImage);

private:
    QObject* m_videoOutput;
    QVideoSink* m_videoSink;
    QVideoFrame m_frameBuffers[2];
};

#endif // MPLAYER_H
