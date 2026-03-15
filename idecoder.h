#ifndef IDECODER_H
#define IDECODER_H

#include <cstdint>
#include <span>
#include <QVideoFrame>
#include <QVideoFrameFormat>
#include <QImage>

class IDecoder
{
public:
    virtual ~IDecoder() = default;

    virtual QVideoFrame& decode(const uint8_t* h264Data, size_t size) = 0;

    virtual int width() const = 0;
    virtual int height() const = 0;
};

#endif // IDECODER_H
