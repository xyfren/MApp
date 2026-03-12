#pragma once

#include <QObject>
#include <QVideoFrame>
#include <QVideoFrameFormat>
#include <vector>
#include <unordered_map>
#include <cstdint>

#include "ffmpegdecoder.h"
#include "spacket.h"

enum class DecoderType {
    FFmpeg,
    Jpeg
};

class FrameManager : public QObject
{
    Q_OBJECT

public:
    explicit FrameManager(uint16_t width, uint16_t height, DecoderType type = DecoderType::Jpeg, QObject* parent = nullptr);

signals:
    // Emitted when a complete frame has been decoded and is ready for display.
    void frameComplete(const QVideoFrame& frame);

public slots:
    // Receives individual SPacket fragments from the network layer.
    void onSPacketReceived(const QByteArray& data);
private:
    struct PendingFrame {
        uint16_t             totalParts   = 0;
        uint16_t             receivedParts = 0;
        std::vector<uint8_t> data;          // Reassembled H.264 byte stream.
        std::vector<bool>    received;      // Which parts we have already received.
    };

    void processCompleteFrame(uint64_t frameId, PendingFrame& frame);

    std::unique_ptr<IDecoder> m_decoder;

    uint16_t m_width = 0;
    uint16_t m_height = 0;

    std::unordered_map<uint64_t, PendingFrame> m_pendingFrames;

    uint64_t m_lastEmittedFrameId = 0;

    static constexpr size_t MAX_PENDING_FRAMES = 8;
};
