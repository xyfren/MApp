#include "jpegdecoder.h"

#include <QDebug>

JpegDecoder::JpegDecoder(uint16_t width, uint16_t height)
    : m_width(width), m_height(height)
{

}

JpegDecoder::~JpegDecoder()
{
}

QVideoFrame& JpegDecoder::decode(const uint8_t* data, size_t size)
{
    // Парсим сырые JPEG-данные
    QImage img = QImage::fromData(data, size, "JPEG");
    if (img.isNull()) {
        qCritical() << "JpegDecoder: Failed to decode JPEG data";
        return m_videoFrame;
    }

    if (img.format() != QImage::Format_ARGB32) {
        img = img.convertToFormat(QImage::Format_ARGB32);
    }

    if (img.width() != m_width || img.height() != m_height) {
        img = img.scaled(m_width, m_height, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    }
    m_videoFrame = QVideoFrame(img);
    // QVideoFrame неявно разделит данные (implicit share) с QImage.
    return m_videoFrame;
}
