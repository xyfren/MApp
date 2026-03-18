#include "jpegdecoder.h"

#include <QDebug>
#include "TimeProfiler.h"

JpegDecoder::JpegDecoder(uint16_t width, uint16_t height)
    : m_width(width), m_height(height)
{
    m_img = QImage(QSize(width,height),QImage::Format_ARGB32);
}

JpegDecoder::~JpegDecoder()
{
}

QVideoFrame JpegDecoder::decode(const uint8_t* data, size_t size)
{
    // Парсим сырые JPEG-данные
    if (!m_img.loadFromData(data, size, "JPEG")){
        qCritical() << "Error m_img.loadFromData";
    }
    if (m_img.isNull()) {
        qCritical() << "JpegDecoder: Failed to decode JPEG data";
        return m_videoFrame;
    }

    if (m_img.width() != m_width || m_img.height() != m_height) {
        m_img = m_img.scaled(m_width, m_height, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    }

    return QVideoFrame(m_img);
}
