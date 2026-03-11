#include "mplayer.h"
#include "QElapsedTimer"

MPlayer::MPlayer(QObject* parent):
    QObject(parent),
    m_videoOutput(nullptr),
    m_videoSink(nullptr)
{
    MClient* client = MClient::instance();
    connect(client,&MClient::frameReceived,this,&MPlayer::onFrameReceived);
}
MPlayer::~MPlayer(){

}

QObject* MPlayer::videoOutput() const {
    return m_videoOutput;
}

void MPlayer::setVideoOutput(QObject* videoOutput) {

    m_videoSink = qvariant_cast<QVideoSink*>(videoOutput->property("videoSink"));
    qDebug() << "m_videoSink: " << m_videoSink;

    QImage image(640, 480, QImage::Format_RGB888);
    image.fill(QColor(255, 192, 203)); // Розовый цвет (RGB)

    // Конвертируем QImage в QVideoFrame
    QVideoFrame frame = QVideoFrame(image);

    // Отправляем в videoSink
    m_videoSink->setVideoFrame(frame);

    m_videoOutput = videoOutput;
}

// void MPlayer::onFrameReceived(uint64_t frameId, const QByteArray& frameData){
//     qDebug() << "New frame in MPlayer: " << frameId;

//     int bufIdx = frameId % 2;

//     m_imageBuffers[bufIdx].loadFromData(frameData,"JPG");
//     m_frameBuffers[bufIdx] = QVideoFrame(m_imageBuffers[bufIdx]);

//     m_videoSink->setVideoFrame(m_frameBuffers[bufIdx]);

// }

void MPlayer::onFrameReceived(const QVideoFrame& frameImage) {
    m_videoSink->setVideoFrame(frameImage);
}
