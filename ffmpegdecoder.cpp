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
    // const AVCodec* codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    const AVCodec* codec = avcodec_find_decoder(AV_CODEC_ID_H264);
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

    m_codecCtx->pix_fmt = AV_PIX_FMT_YUV420P;

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

QVideoFrame FFmpegDecoder::decode(const uint8_t* h264Data, size_t size)
{
    QVideoFrameFormat videoFrameFormat(QSize(m_width, m_height), QVideoFrameFormat::Format_YUV420P);
    QVideoFrame videoFrame(videoFrameFormat);

    // qDebug() << "aaa";
    if (!m_codecCtx || !m_decodedFrame || !m_packet) {
        return videoFrame;
    }

    m_packet->data = const_cast<uint8_t*>(h264Data);
    m_packet->size = static_cast<int>(size);
    // qDebug() << "bbb";

    int ret = avcodec_send_packet(m_codecCtx, m_packet);
    if (ret < 0) {
        char errbuf[128];
        av_strerror(ret, errbuf, sizeof(errbuf));
        qCritical() << "FFmpegDecoder: avcodec_send_packet error:" << errbuf << "Size:" << size;
        return videoFrame;
    }

    ret = avcodec_receive_frame(m_codecCtx, m_decodedFrame);
    if (ret < 0) {
        if (ret != AVERROR(EAGAIN)) { // EAGAIN - это норма, значит нужно больше данных
            char errbuf[128];
            av_strerror(ret, errbuf, sizeof(errbuf));
            qCritical() << "FFmpegDecoder: avcodec_receive_frame error:" << errbuf;
        }
        return videoFrame;
    }
    // qDebug() << "ddd";
    const int w = m_decodedFrame->width;
    const int h = m_decodedFrame->height;



    if (!videoFrame.map(QVideoFrame::WriteOnly)) {
        qCritical() << "FFmpegDecoder: Failed to map QVideoFrame";
        return videoFrame;
    }

    for (int plane = 0; plane < 3; ++plane) {
        const uint8_t* src = m_decodedFrame->data[plane];
        uint8_t* dst = videoFrame.bits(plane);

        const int srcStride = m_decodedFrame->linesize[plane];
        const int dstStride = videoFrame.bytesPerLine(plane);

        // Правильный расчет размеров для каждой плоскости
        const int planeWidth = (plane == 0) ? w : w / 2;  // ширина в пикселях
        const int planeHeight = (plane == 0) ? h : h / 2;

        if (srcStride == dstStride) {
            // Если строки совпадают - копируем всё
            std::memcpy(dst, src, static_cast<size_t>(planeHeight) * srcStride);
        } else {
            // Копируем построчно
            for (int y = 0; y < planeHeight; ++y) {
                std::memcpy(dst + y * dstStride,
                            src + y * srcStride,
                            planeWidth);  // planeWidth уже в байтах!
            }
        }
    }
    videoFrame.unmap();
    return videoFrame;
}
