#include "jpegdecoder.h"

#include <QDebug>

JpegDecoder::JpegDecoder(uint16_t width, uint16_t height)
    : m_width(width), m_height(height)
{
}

JpegDecoder::~JpegDecoder()
{
}

std::span<const uint8_t> JpegDecoder::decode(const uint8_t* data, size_t size)
{
    // Парсим сырые JPEG-данные
    QImage img = QImage::fromData(data, size, "JPEG");
    if (img.isNull()) {
        qCritical() << "JpegDecoder: Failed to decode JPEG data";
        return {};
    }

    // Приводим к 32-битному формату (BGRA в памяти)
    if (img.format() != QImage::Format_ARGB32) {
        img = img.convertToFormat(QImage::Format_ARGB32);
    }

    // Если размеры вдруг отличаются от целевых, скейлим.
    if (img.width() != m_width || img.height() != m_height) {
        img = img.scaled(m_width, m_height, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    }

    // Сохраняем QImage, чтобы память не очистилась при выходе из скоупа
    m_lastImage = std::move(img);

    // Возвращаем span на внутренний буфер
    return { m_lastImage.constBits(), static_cast<size_t>(m_lastImage.sizeInBytes()) };
}
