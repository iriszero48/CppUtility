#pragma once

#ifdef CU_IMG_HAS_QT
#include "IContext.hpp"

#include <QImage>

namespace CuImg
{
    namespace Detail::QT
    {
        template <typename T>
        struct ToQtFormat
        {
        };
        template <>
        struct ToQtFormat<CuRGB>
        {
            static constexpr QImage::Format Format() { return QImage::Format::Format_RGB888; }
        };
        template <>
        struct ToQtFormat<CuRGBA>
        {
            static constexpr QImage::Format Format() { return QImage::Format::Format_RGBA8888; }
        };
    }

    template <typename T>
    struct QtImageContext : IImageContext<QtImageContext<T>, T>
    {
        struct LoadInfo
        {

        } Info{};

        struct SaveParam
        {

        } Param{};

        QImage Img{};

        void Create(const size_t width, const size_t height)
        {
            CuUtil_Assert(width < std::numeric_limits<int>::max() && height < std::numeric_limits<int>::max(), QtException);

            constexpr auto Format = Detail::QT::ToQtFormat<T>::Format();
            Img = QImage(width, height, Format);
        }

        [[nodiscard]] uint8_t *Data() { return Img.bits(); }
        [[nodiscard]] const uint8_t *Data() const { return Img.bits(); }
        [[nodiscard]] size_t Width() const { return Img.width(); }
        [[nodiscard]] size_t Height() const { return Img.height(); }
        [[nodiscard]] size_t Size() const { return Img.sizeInBytes(); }
        [[nodiscard]] size_t Count() const { return Width() * Height(); }
        [[nodiscard]] size_t Linesize() const { return Img.bytesPerLine(); }
        [[nodiscard]] bool Empty() const { return Img.isNull(); }

        [[nodiscard]] const QImage &Raw() const { return Img; }
        [[nodiscard]] QImage &Raw() { return Img; }

        [[nodiscard]] const LoadInfo &GetInfo() const { return Info; }
        [[nodiscard]] LoadInfo &GetInfo() { return Info; }
        [[nodiscard]] const SaveParam &GetParam() const { return Param; }
        [[nodiscard]] SaveParam &GetParam() { return Param; }
    };
}
#endif
