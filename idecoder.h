#ifndef IDECODER_H
#define IDECODER_H

#include <cstdint>
#include <span>

class IDecoder
{
public:
    virtual ~IDecoder() = default;

    virtual std::span<const uint8_t> decode(const uint8_t* data, size_t size) = 0;

    virtual int width() const = 0;
    virtual int height() const = 0;
};

#endif // IDECODER_H
