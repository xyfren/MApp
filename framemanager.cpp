#include "framemanager.h"

#include <QImage>
#include <QVideoFrame>
#include <QVideoFrameFormat>
#include <algorithm>
#include <cstring>
#include <iostream>
#include <QDebug>

#include "jpegdecoder.h"
#include "ffmpegdecoder.h"

FrameManager::FrameManager(uint16_t width, uint16_t height, CoderType type, QObject* parent)
    : QObject(parent)
    , m_width(width)
    , m_height(height)
{
    for (auto& buf : m_buffers) {
        buf.data.resize(PREALLOCATED_SIZE);
    }
    setDecoderType(type);
}

void FrameManager::setDecoderType(CoderType type)
{
    std::lock_guard<std::mutex> lock(m_decoderMutex);
    if (type == CoderType::Jpeg) {
        m_decoder = std::make_unique<JpegDecoder>(m_width, m_height);
    }
    else if (type == CoderType::FFmpeg) {
        m_decoder = std::make_unique<FFmpegDecoder>(m_width, m_height);
    }
}

void FrameManager::onSPacketReceived(const QByteArray& data) {
    const SPacket* packet = reinterpret_cast<const SPacket*>(data.data());
    const uint64_t frameId = packet->frameId;

    if (frameId <= m_lastEmittedFrameId) return;

    PendingFrame& pending = m_buffers[frameId % NUM_BUFFERS];

    // Если в слоте старый кадр — инициализируем под новый
    if (pending.currentFrameId != frameId) {
        if (pending.currentFrameId > frameId) return;
        pending.setup(frameId, packet->totalParts);
    }

    // Просто копируем данные по офсету
    std::memcpy(pending.data.data() + packet->dataOffset, packet->data, packet->dataSize);
    pending.receivedParts++;

    // Запоминаем размер всего кадра, когда пришла его последняя часть
    if (packet->partId == packet->totalParts - 1) {
        pending.actualSize = packet->dataOffset + packet->dataSize;
    }

    // Простая проверка по счетчику
    if (pending.receivedParts == pending.totalParts) {
        // Если actualSize еще не был установлен (последняя часть пришла не последней),
        // вычисляем его по формуле, если размеры пакетов фиксированы
        if (pending.actualSize == 0) {
            pending.actualSize = (pending.totalParts - 1) * SPACKET_MAX_DATA_SIZE + packet->dataSize;
        }
        // qDebug() << "ggg";
        processCompleteFrame(frameId, pending);
        m_lastEmittedFrameId = frameId;
    }
}

void FrameManager::processCompleteFrame(uint64_t frameId, PendingFrame& frame)
{
    // Защищаем декодер, так как он может быть заменен в setDecoderType из другого потока
    // std::lock_guard<std::mutex> lock(m_decoderMutex);
    if (!m_decoder) {
        return;
    }

    // Передаем точный (totalBytes), а не выделенный (data.size()) размер!
    QVideoFrame videoFrame = m_decoder->decode(frame.data.data(), frame.actualSize);

    if (videoFrame.isValid()) {
        emit frameComplete(videoFrame);
    } else {
        qDebug() << "FrameManager: Decoder returned invalid frame for ID" << frameId;
    }
}