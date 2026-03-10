#include "framemanager.h"
#include <cstring>
#include <QDebug>

FrameManager::FrameManager(QObject *parent) : QObject(parent) {
    // Резервируем память заранее, чтобы избежать аллокаций при приеме пакетов
    m_buffers[0].data.reserve(4 * 4096 * 4096); // 2 MB под кадр
    m_buffers[1].data.reserve(4 * 4096 * 4096);
}

void FrameManager::onFPacketReceived(const QByteArray data) {
    const FPacket* packet = reinterpret_cast<const FPacket*>(data.constData());
    // Определяем, в какой буфер должен попасть пакет
    uint32_t fId = packet->frameId;
    int idx = fId % 2;
    FrameBuffer &buf = m_buffers[idx];

    // Если пришел пакет от НОВОГО кадра (например, был 2, пришел 4)
    if (buf.currentFrameId != fId) {
        buf.currentFrameId = fId;
        buf.partsReceived = 0;
        buf.totalParts = packet->totalParts;
    }

    // 1. Рассчитываем необходимый размер буфера на основе смещения
    int requiredSize = packet->partOffset + packet->partSize;
    if (buf.data.size() < requiredSize) {
        buf.data.resize(requiredSize);
    }

    // 2. Копируем данные в нужную позицию
    std::memcpy(buf.data.data() + packet->partOffset,
                packet->partData,
                packet->partSize);

    buf.partsReceived++;

    // 3. Если собрали все части — отправляем на обработку
    if (buf.partsReceived >= buf.totalParts && fId > m_lastFrame) {

        if (fId - m_lastFrame > 1){
            qDebug() << "Были потери: fId: " << fId << "; lastFrame: " << m_lastFrame;
        }
        m_lastFrame = fId;
        m_buffers[idx].image.loadFromData(buf.data,"JPG");
        m_buffers[idx].frame = QVideoFrame(m_buffers[idx].image);
        emit frameComplete(fId, m_buffers[idx].frame);

        // // Помечаем буфер как "свободный" или "завершенный",
        // // чтобы не обрабатывать повторно, если пакеты дублируются
        // buf.currentFrameId = 0xFFFFFFFF;
    }
}
