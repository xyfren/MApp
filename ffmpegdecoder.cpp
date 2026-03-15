#include "ffmpegdecoder.h"

#include <iostream>

FFmpegDecoder::FFmpegDecoder(uint16_t width, uint16_t height)
    : m_width(width), m_height(height)
{
    QVideoFrameFormat videoFrameFormat(QSize(width,height),QVideoFrameFormat::Format_NV12);
    QVideoFrame videoFrame(videoFrameFormat);
    m_videoFrame = videoFrame;

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

    m_codecCtx->pix_fmt = AV_PIX_FMT_NV12;

    // Настройки для Low Latency
    m_codecCtx->flags |= AV_CODEC_FLAG_LOW_DELAY;
    m_codecCtx->flags2 |= AV_CODEC_FLAG2_FAST;
    m_codecCtx->thread_count = 1;

    if (avcodec_open2(m_codecCtx, codec, nullptr) < 0) {
        qCritical() << "FFmpegDecoder: avcodec_open2 failed";
        return false;
    }

    m_decodedFrame = av_frame_alloc();
    if (!m_decodedFrame) return false;

    m_packet = av_packet_alloc();
    if (!m_packet) return false;

    return true;
}

void FFmpegDecoder::cleanup()
{
    if (m_swsCtx)       { sws_freeContext(m_swsCtx);        m_swsCtx       = nullptr; }
    if (m_packet)       { av_packet_free(&m_packet);         m_packet       = nullptr; }
    if (m_decodedFrame) { av_frame_free(&m_decodedFrame);    m_decodedFrame = nullptr; }
    if (m_codecCtx)     { avcodec_free_context(&m_codecCtx); m_codecCtx     = nullptr; }
}

QVideoFrame& FFmpegDecoder::decode(const uint8_t* h264Data, size_t size)
{


    if (!m_codecCtx || !m_decodedFrame || !m_packet) {
        return m_videoFrame;
    }

    m_packet->data = const_cast<uint8_t*>(h264Data);
    m_packet->size = static_cast<int>(size);

    int ret = avcodec_send_packet(m_codecCtx, m_packet);
    if (ret < 0) {
        qCritical() << "FFmpegDecoder: avcodec_send_packet failed (" << ret << ")";
        return m_videoFrame;
    }

    ret = avcodec_receive_frame(m_codecCtx, m_decodedFrame);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
        // Ожидаемое поведение для неполных кадров
        return m_videoFrame;
    } else if (ret < 0) {
        qCritical() << "FFmpegDecoder: avcodec_receive_frame failed (" << ret << ")";
        return m_videoFrame;
    }

    const int w = m_decodedFrame->width;
    const int h = m_decodedFrame->height;



    if (!m_videoFrame.map(QVideoFrame::WriteOnly)) {
        qCritical() << "FFmpegDecoder: Failed to map QVideoFrame";
        return m_videoFrame;
    }

    for (int plane = 0; plane < 2; ++plane) {
        const uint8_t* src = m_decodedFrame->data[plane];
        uint8_t* dst = m_videoFrame.bits(plane);
        const int srcStride = m_decodedFrame->linesize[plane];
        const int dstStride = m_videoFrame.bytesPerLine(plane);

        const int planeHeight = (plane == 0) ? h : h / 2;
        const int planeWidthInBytes = w;

        if (srcStride == dstStride) {
            std::memcpy(dst, src, static_cast<size_t>(planeHeight) * srcStride);
        } else {
            for (int y = 0; y < planeHeight; ++y) {
                std::memcpy(dst + y * dstStride, src + y * srcStride, planeWidthInBytes);
            }
        }
    }

    m_videoFrame.unmap();
    return m_videoFrame;
}
