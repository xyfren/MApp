#include "ffmpegdecoder.h"

#include <iostream>

FFmpegDecoder::FFmpegDecoder(uint16_t width, uint16_t height)
    : m_width(width), m_height(height)
{
    if (!initialize()) {
        qCritical() << "FFmpegDecoder: initialization failed";
    }
}

FFmpegDecoder::~FFmpegDecoder()
{
    cleanup();
}

bool FFmpegDecoder::initialize()
{
    const AVCodec* codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    // const AVCodec* codec = avcodec_find_decoder_by_name("h264_mediacodec");
    if (!codec) {
        qCritical() << "FFmpegDecoder: H.264 decoder not found";
        return false;
    }

    m_codecCtx = avcodec_alloc_context3(codec);
    if (!m_codecCtx) {
        qCritical() << "FFmpegDecoder: avcodec_alloc_context3 failed";
        return false;
    }

    // Match encoder settings for low-latency single-threaded slice decoding.
    m_codecCtx->width       = m_width;
    m_codecCtx->height      = m_height;
    m_codecCtx->pix_fmt     = AV_PIX_FMT_YUV420P;
    m_codecCtx->thread_type = FF_THREAD_SLICE;

    // Enable low-latency flags: output frames as soon as possible.
    m_codecCtx->flags  |= AV_CODEC_FLAG_LOW_DELAY;
    m_codecCtx->flags2 |= AV_CODEC_FLAG2_FAST;

    if (avcodec_open2(m_codecCtx, codec, nullptr) < 0) {
        qCritical() << "FFmpegDecoder: avcodec_open2 failed";
        return false;
    }

    m_decodedFrame = av_frame_alloc();
    if (!m_decodedFrame) return false;

    m_packet = av_packet_alloc();
    if (!m_packet) return false;

    // Color-space conversion context: YUV420P (decoder output) -> BGRA (display).
    m_swsCtx = sws_getContext(
        m_width,  m_height, AV_PIX_FMT_YUV420P,
        m_width,  m_height, AV_PIX_FMT_BGRA,
        SWS_BILINEAR, nullptr, nullptr, nullptr);
    if (!m_swsCtx) {
        qCritical() << "FFmpegDecoder: sws_getContext failed";
        return false;
    }

    // Pre-allocate BGRA output buffer (tightly packed, no padding).
    const size_t bgraSize = static_cast<size_t>(m_width) * m_height * 4;
    m_bgraBuffer.resize(bgraSize);

    return true;
}

void FFmpegDecoder::cleanup()
{
    if (m_swsCtx)       { sws_freeContext(m_swsCtx);        m_swsCtx       = nullptr; }
    if (m_packet)       { av_packet_free(&m_packet);         m_packet       = nullptr; }
    if (m_decodedFrame) { av_frame_free(&m_decodedFrame);    m_decodedFrame = nullptr; }
    if (m_codecCtx)     { avcodec_free_context(&m_codecCtx); m_codecCtx     = nullptr; }
}

std::span<const uint8_t> FFmpegDecoder::decode(const uint8_t* h264Data, size_t size)
{
    if (!m_codecCtx || !m_decodedFrame || !m_swsCtx || !m_packet) {
        return {};
    }

    // Wrap the incoming H.264 data in an AVPacket (no copy — just pointer + size).
    m_packet->data = const_cast<uint8_t*>(h264Data);
    m_packet->size = static_cast<int>(size);

    // Submit the packet to the decoder.
    int ret = avcodec_send_packet(m_codecCtx, m_packet);
    if (ret < 0) {
        qCritical() << "FFmpegDecoder: avcodec_send_packet failed (" << ret << ")";
        return {};
    }

    // Retrieve the decoded YUV420P frame.
    ret = avcodec_receive_frame(m_codecCtx, m_decodedFrame);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
        // No frame available yet (shouldn't happen with intra-only, but be safe).
        return {};
    }
    if (ret < 0) {
        qCritical() << "FFmpegDecoder: avcodec_receive_frame failed (" << ret << ")";
        return {};
    }

    // Convert YUV420P -> BGRA into our pre-allocated buffer.
    uint8_t* dstSlice[1]  = { m_bgraBuffer.data() };
    int      dstStride[1] = { m_width * 4 };
    sws_scale(m_swsCtx,
              m_decodedFrame->data, m_decodedFrame->linesize,
              0, m_height,
              dstSlice, dstStride);

    return { m_bgraBuffer.data(), m_bgraBuffer.size() };
}
