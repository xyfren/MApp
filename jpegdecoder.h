#ifndef JPEGDECODER_H
#define JPEGDECODER_H

#include <QImage>
#include <cstdint>
#include <span>
#include "idecoder.h"

class JpegDecoder: public IDecoder
{
public:
    explicit JpegDecoder(uint16_t width, uint16_t height);
    ~JpegDecoder() override;

    QVideoFrame decode(const uint8_t* data, size_t size) override;

    int width() const override { return m_width; }
    int height() const override { return m_height; }

private:
    uint16_t m_width;
    uint16_t m_height;

    // QImage сам управляет памятью. Храним его как член класса,
    // чтобы span, который мы возвращаем из decode(), оставался валидным.
    QImage m_lastImage;
    QVideoFrame m_videoFrame;
};

#endif
