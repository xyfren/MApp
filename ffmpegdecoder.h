#ifndef FFMPEGCODER_H
#define FFMPEGCODER_H

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

#include <vector>
#include <QDebug>
#include "idecoder.h"

class FFmpegDecoder : public IDecoder
{
public:
    explicit FFmpegDecoder(uint16_t width, uint16_t height);
    ~FFmpegDecoder() override;

    std::span<const uint8_t> decode(const uint8_t* h264Data, size_t size) override;

    int width()  const override { return m_width;  }
    int height() const override { return m_height; }

private:
    bool initialize();
    void cleanup();

    uint16_t m_width = 0;
    uint16_t m_height = 0;
    AVCodecContext *m_codecCtx = nullptr;
    AVFrame*        m_decodedFrame = nullptr;   // YUV420P frame from decoder
    AVPacket*       m_packet   = nullptr;
    SwsContext*     m_swsCtx   = nullptr;

    // Tightly-packed BGRA output buffer (width * height * 4 bytes).
    // Reused across decode() invocations to avoid per-frame heap allocation.
    std::vector<uint8_t> m_bgraBuffer;
};

#endif
