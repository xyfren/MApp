#pragma once

#include <QObject>
#include <QVideoFrame>
#include <QVideoFrameFormat>
#include <vector>
#include <array>
#include <cstdint>
#include <mutex>
#include <memory>

#include <mappsettings.h>
#include "ffmpegdecoder.h"
#include "spacket.h"

class FrameManager : public QObject
{
    Q_OBJECT

public:
    explicit FrameManager(uint16_t width, uint16_t height, CoderType type = CoderType::Null, QObject* parent = nullptr);

signals:
    // Emitted when a complete frame has been decoded and is ready for display.
    void frameComplete(const QVideoFrame& frame);

public slots:
    void setDecoderType(CoderType type);
    // Receives individual SPacket fragments from the network layer.
    void onSPacketReceived(const QByteArray& data);

private:
    struct PendingFrame {
        uint64_t currentFrameId = 0;
        uint16_t receivedParts = 0;
        uint16_t totalParts = 0;
        size_t   actualSize = 0;

        // Выделяем память один раз (например, 2MB, если это типичный предел для ваших кадров)
        // Либо используйте (SPACKET_MAX_DATA_SIZE * MAX_EXPECTED_PARTS)
        std::vector<uint8_t> data;

        void setup(uint64_t frameId, uint16_t parts) {
            currentFrameId = frameId;
            totalParts = parts;
            receivedParts = 0;
            actualSize = 0;
            // Память внутри data не трогаем, просто будем в нее писать
        }
    };

    void processCompleteFrame(uint64_t frameId, PendingFrame& frame);

    std::unique_ptr<IDecoder> m_decoder;
    std::mutex m_decoderMutex;

    uint16_t m_width = 0;
    uint16_t m_height = 0;

    // 10 статических буферов вместо динамической unordered_map
    static constexpr size_t NUM_BUFFERS = 10;
    static constexpr size_t PREALLOCATED_SIZE = 1024 * 1024 * 2; // 2 MB
    std::array<PendingFrame, NUM_BUFFERS> m_buffers;

    uint64_t m_lastEmittedFrameId = 0;
};