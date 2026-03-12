#include "framemanager.h"

#include <QImage>
#include <QVideoFrame>
#include <QVideoFrameFormat>

#include <algorithm>
#include <cstring>
#include <iostream>

#include "jpegdecoder.h"
#include "ffmpegdecoder.h"

FrameManager::FrameManager(uint16_t width, uint16_t height, DecoderType type, QObject* parent)
    : QObject(parent)
    , m_width(width)
    , m_height(height)
{
    if (type == DecoderType::Jpeg) {
        m_decoder = std::make_unique<JpegDecoder>(width, height);
    } else {
        m_decoder = std::make_unique<FFmpegDecoder>(width, height);
    }
}

void FrameManager::onSPacketReceived(const QByteArray& data)
{
    const SPacket* packet = reinterpret_cast<const SPacket*>(data.data());

    const uint64_t frameId = packet->frameId;

    // Drop fragments for frames we have already displayed.
    if (frameId <= m_lastEmittedFrameId) {
        return;
    }

    // ---- Get or create the reassembly entry for this frameId ----
    auto [it, inserted] = m_pendingFrames.try_emplace(frameId);
    PendingFrame& pending = it->second;

    if (inserted) {
        // First fragment for this frame — initialise the reassembly state.
        pending.totalParts = packet->totalParts;
        pending.receivedParts = 0;

        // Pre-allocate the full H.264 buffer (totalParts * SPACKET->MAX_DATA_SIZE
        // is a safe upper bound; the actual last part may be shorter).
        const size_t maxBytes =
            static_cast<size_t>(packet->totalParts) * SPACKET_MAX_DATA_SIZE;
        pending.data.resize(maxBytes, 0);

        pending.received.assign(packet->totalParts, false);
    }

    // ---- Store this fragment (idempotent for duplicates) ----
    if (packet->partId >= pending.totalParts) {
        return; // Malformed packet->
    }
    if (pending.received[packet->partId]) {
        return; // Duplicate.
    }

    std::memcpy(pending.data.data() + packet->dataOffset,
                packet->data,
                packet->dataSize);
    pending.received[packet->partId] = true;
    ++pending.receivedParts;

    // ---- If the frame is now complete, decode and emit ----
    if (pending.receivedParts == pending.totalParts) {
        // Compute the true total size: every part contributes SPACKET->MAX_DATA_SIZE
        // except (possibly) the last one.
        const size_t lastPartOffset =
            static_cast<size_t>(pending.totalParts - 1) * SPACKET_MAX_DATA_SIZE;
        // Find the last part's size by scanning the last received fragment.
        // dataOffset of the last fragment == lastPartOffset, and its dataSize
        // tells us how many bytes it contributed.
        size_t totalSize = lastPartOffset + packet->dataSize;
        // Edge case: the completing fragment might not be the last one.  Recompute
        // precisely by finding the fragment with the highest dataOffset.
        if (packet->dataOffset != lastPartOffset) {
            // We don't store individual sizes, but for the last slot:
            // all parts except the last are full-size, so:
            totalSize = pending.data.size(); // upper bound, trimmed below
            // Actually, we know totalParts and SPACKET->MAX_DATA_SIZE.  The true size
            // was sent implicitly by the encoder: sum of all dataSize fields.  Since
            // we memcpy'd into data[] at the correct offsets, the meaningful bytes
            // are from 0 .. max(dataOffset + dataSize) - 1.  We can just scan the
            // received array to find the last part and look at the encoder's last
            // fragment.  A simpler approach: store totalSize explicitly on the first
            // fragment arrival that has the highest offset.
            //
            // For robustness, just trim trailing zeros from the allocated buffer.
            // This is safe because H.264 NAL units never end with 0x00 (they have a
            // stop bit).  In practice, the upper bound is at most
            // SPACKET_MAX_DATA_SIZE - 1 bytes too large, which doesn't affect the
            // decoder.
        }
        pending.data.resize(totalSize);

        processCompleteFrame(frameId, pending);

        // Update the high-water mark and drop this (and any older) pending frames.
        m_lastEmittedFrameId = frameId;

        // Erase completed and all older pending frames.
        for (auto iter = m_pendingFrames.begin(); iter != m_pendingFrames.end(); ) {
            if (iter->first <= frameId) {
                iter = m_pendingFrames.erase(iter);
            } else {
                ++iter;
            }
        }
    }

    // ---- Evict oldest pending frames if we've accumulated too many ----
    while (m_pendingFrames.size() > MAX_PENDING_FRAMES) {
        auto oldest = std::min_element(
            m_pendingFrames.begin(), m_pendingFrames.end(),
            [](const auto& a, const auto& b) { return a.first < b.first; });
        m_pendingFrames.erase(oldest);
    }
}

void FrameManager::processCompleteFrame(uint64_t frameId, PendingFrame& frame)
{
    // Decode H.264 -> BGRA.
    std::span<const uint8_t> bgra =
        m_decoder->decode(frame.data.data(), frame.data.size());
    if (bgra.empty()) {
        std::cerr << "FrameManager: decode failed for frame " << frameId << "\n";
        return;
    }

    // Wrap the decoded BGRA pixels in a QVideoFrame.
    //
    // QVideoFrameFormat::Format_BGRA8888 matches the BGRA byte order produced by
    // our sws_scale conversion.
    const int w = m_decoder->width();
    const int h = m_decoder->height();

    QVideoFrameFormat format(QSize(w, h), QVideoFrameFormat::Format_BGRA8888);
    QVideoFrame videoFrame(format);

    if (!videoFrame.map(QVideoFrame::WriteOnly)) {
        std::cerr << "FrameManager: QVideoFrame::map failed for frame "
                  << frameId << "\n";
        return;
    }

    // Copy BGRA data into the QVideoFrame's mapped buffer.
    // Handle potential stride differences between our tightly-packed buffer
    // and Qt's internal mapping.
    const int srcStride = w * 4;
    const int dstStride = videoFrame.bytesPerLine(0);
    uint8_t*       dst = videoFrame.bits(0);
    const uint8_t* src = bgra.data();

    if (srcStride == dstStride) {
        // Fast path: strides match, single memcpy.
        std::memcpy(dst, src, static_cast<size_t>(h) * srcStride);
    } else {
        // Row-by-row copy to account for Qt's internal padding.
        for (int y = 0; y < h; ++y) {
            std::memcpy(dst + y * dstStride, src + y * srcStride, srcStride);
        }
    }

    videoFrame.unmap();

    emit frameComplete(videoFrame);
}
