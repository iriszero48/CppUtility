#pragma once

#include "Image.hpp"

namespace CuImg
{
    namespace Detail
    {
        template <Backend DestB, typename T, Backend SrcB>
        void ConvertImageByLine(const Image<T, SrcB> &src, Image<T, DestB> &dest)
        {
            const auto srcW = src.Width();
            const auto srcH = src.Height();
            const auto srcLinesize = src.Linesize();

            if (srcW != dest.Width() || srcH != dest.Height())
            {
                dest.Create(srcW, srcH);
            }

            const auto destW = dest.Width();
            const auto destH = dest.Height();
            const auto destLinesize = dest.Linesize();

            const auto usedLinesize = destW * T::ColorSize();

            if (srcLinesize == destLinesize)
            {
                std::copy_n(src.Data(), dest.Size(), dest.Data());
            }
            else
            {
                for (size_t i = 0; i < destH; ++i)
                {
                    std::copy_n(src.Data() + srcLinesize * i, usedLinesize, dest.Data() + destLinesize * i);
                }
            }
        }

        template <typename Src, typename Dest>
        void ConvertImageByPixel(const Src &src, Dest &dest)
        {
            if (src.Width() != dest.Width() || src.Height() != dest.Height())
            {
                dest.Create(src.Width(), src.Height());
            }

            auto sit = src.begin();
            auto dit = dest.begin();
            auto dEnd = dest.end();
            for (; dit != dEnd; ++sit, ++dit)
            {
                const decltype(Src{}.At(0, 0)) raw = *sit;
                decltype(Dest{}.At(0, 0)) pix{};
                Color::Converter<std::remove_cv_t<decltype(raw)>, decltype(pix)>{}(raw, pix);
                *dit = pix;
            }
        }

    }

    template <Backend DestB, typename T, Backend SrcB>
    void Convert(const Image<T, SrcB> &src, Image<T, DestB> &dest)
    {
        Detail::ConvertImageByLine(src, dest);
    }

    template <typename SrcT, Backend SrcB, typename DestT, Backend DestB>
    void Convert(const Image<SrcT, SrcB> &src, Image<DestT, DestB> &dest)
    {
        Detail::ConvertImageByPixel(src, dest);
    }

    template <typename SrcT, Backend SrcB, typename DestT, Backend DestB>
    void Convert(const DiscreteImage<SrcT, SrcB> &src, Image<DestT, DestB> &dest)
    {
        Detail::ConvertImageByPixel(src, dest);
    }

    template <typename SrcT, Backend SrcB, typename DestT, Backend DestB>
    void Convert(const Image<SrcT, SrcB> &src, DiscreteImage<DestT, DestB> &dest)
    {
        Detail::ConvertImageByPixel(src, dest);
    }

    template <typename SrcT, Backend SrcB, typename DestT, Backend DestB>
    void Convert(const DiscreteImage<SrcT, SrcB> &src, DiscreteImage<DestT, DestB> &dest)
    {
        Detail::ConvertImageByPixel(src, dest);
    }

    template <typename DestT, Backend DestB, typename SrcT, Backend SrcB>
    Image<DestT, DestB> ConvertToImage(const Image<SrcT, SrcB> &img)
    {
        Image<DestT, DestB> ret{};
        Convert(img, ret);
        return ret;
    }

    template <typename DestT, Backend DestB, typename SrcT, Backend SrcB>
    Image<DestT, DestB> ConvertToImage(const DiscreteImage<SrcT, SrcB> &img)
    {
        Image<DestT, DestB> ret{};
        Convert(img, ret);
        return ret;
    }

    template <typename DestT, Backend DestB, typename SrcT, Backend SrcB>
    DiscreteImage<DestT, DestB> ConvertToDiscreteImage(const Image<SrcT, SrcB> &img)
    {
        DiscreteImage<DestT, DestB> ret{};
        Convert(img, ret);
        return ret;
    }

    template <typename DestT, Backend DestB, typename SrcT, Backend SrcB>
    DiscreteImage<DestT, DestB> ConvertToDiscreteImage(const DiscreteImage<SrcT, SrcB> &img)
    {
        DiscreteImage<DestT, DestB> ret{};
        Convert(img, ret);
        return ret;
    }

#define MakeConvertBackendImpl(c, be, suffix, Discrete)                                                                                                               \
    template <typename T, Backend B>                                                                                                                                  \
    void Convert_##Discrete##Image##c##suffix(const Image<T, B> &src, Discrete##Image<Cu##c, Backend::be> &dest) { Convert(src, dest); }                              \
    template <typename T, Backend B>                                                                                                                                  \
    void Convert_##Discrete##Image##c##suffix(const DiscreteImage<T, B> &src, Discrete##Image<Cu##c, Backend::be> &dest) { Convert(src, dest); }                      \
    template <typename T, Backend B>                                                                                                                                  \
    Discrete##Image<Cu##c, Backend::be> ConvertTo##Discrete##Image##c##suffix(const Image<T, B> &img) { return ConvertTo##Discrete##Image<Cu##c, Backend::be>(img); } \
    template <typename T, Backend B>                                                                                                                                  \
    Discrete##Image<Cu##c, Backend::be> ConvertTo##Discrete##Image##c##suffix(const DiscreteImage<T, B> &img) { return ConvertTo##Discrete##Image<Cu##c, Backend::be>(img); }

#define MakeConvertBackend(c, be) MakeConvertBackendImpl(c, be, _##be, )
#define MakeConvertDiscreteBackend(c, be) MakeConvertBackendImpl(c, be, _##be, Discrete)

    MakeConvertBackendImpl(RGB, None, , );
    MakeConvertBackendImpl(RGBA, None, , );
    MakeConvertBackendImpl(BGR, None, , );

#ifdef CU_IMG_HAS_STB
    MakeConvertBackend(RGB, STB);
    MakeConvertBackend(RGBA, STB);
#endif

#ifdef CU_IMG_HAS_DIRECTXTEX
    MakeConvertBackend(RGBA, DirectXTex);
#endif

#ifdef CU_IMG_HAS_QT
    MakeConvertBackend(RGB, QT);
    MakeConvertBackend(RGBA, QT);
#endif

#ifdef CU_IMG_HAS_WXWIDGETS
    MakeConvertDiscreteBackend(RGBA, wxWidgets);
#endif

#ifdef CU_IMG_HAS_OPENCV
    MakeConvertBackend(RGB, OpenCV);
    MakeConvertBackend(RGBA, OpenCV);
    MakeConvertBackend(BGR, OpenCV);
#endif

#ifdef CU_IMG_HAS_GRAPHICSMAGICK
    MakeConvertDiscreteBackend(RGBOpacity16, GraphicsMagick);
#endif

#undef MakeConvertBackend
#undef MakeConvertDiscreteBackend
#undef MakeConvertBackendImpl

    namespace Detail
    {
        template <typename T, typename RefImage>
        void ConvertToRefImpl(T *data, size_t width, size_t height, size_t linesize, RefImage &ref)
        {
            ref.GetContext().SetSource(data, width, height, linesize);
        }

        template <typename SrcImage, typename RefImage>
        void ConvertToRefImpl(SrcImage &src, RefImage &ref)
        {
            ref.GetContext().SetSource(src.Data(), src.Width(), src.Height(), src.Linesize());
        }
    }

    template <typename T>
    void ConvertToRef(typename T::Type *data, const size_t width, const size_t height, const size_t linesize, Image<T, Backend::Ref> &ref)
    {
        Detail::ConvertToRefImpl<typename T::Type, Image<T, Backend::Ref>>(data, width, height, linesize, ref);
    }

    template <typename T>
    Image<T, Backend::Ref> ConvertToRef(typename T::Type *data, size_t width, size_t height, size_t linesize)
    {
        Image<T, Backend::Ref> ret;
        ConvertToRef(data, width, height, linesize, ret);
        return ret;
    }

    template <typename T, Backend Src>
    void ConvertToRef(Image<T, Src> &src, Image<T, Backend::Ref> &ref)
    {
        Detail::ConvertToRefImpl(src, ref);
    }

    template <typename T, Backend Src>
    Image<T, Backend::Ref> ConvertToRef(Image<T, Src> &src)
    {
        Image<T, Backend::Ref> ret{};
        ConvertToRef(src, ret);
        return ret;
    }

    template <typename T>
    void ConvertToConstRef(const typename T::Type *data, const size_t width, const size_t height, const size_t linesize, Image<T, Backend::ConstRef> &ref)
    {
        Detail::ConvertToRefImpl<const typename T::Type, Image<T, Backend::ConstRef>>(data, width, height, linesize, ref);
    }

    template <typename T>
    Image<T, Backend::ConstRef> ConvertToConstRef(const typename T::Type *data, size_t width, size_t height, size_t linesize)
    {
        Image<T, Backend::ConstRef> ret;
        ConvertToConstRef(data, width, height, linesize, ret);
        return ret;
    }

    template <typename T, Backend Src>
    void ConvertToConstRef(const Image<T, Src> &src, Image<T, Backend::ConstRef> &ref)
    {
        Detail::ConvertToRefImpl(src, ref);
    }

    template <typename T, Backend Src>
    Image<T, Backend::ConstRef> ConvertToConstRef(const Image<T, Src> &src)
    {
        Image<T, Backend::ConstRef> ret{};
        ConvertToConstRef(src, ret);
        return ret;
    }

#define MakeConvertRawToRef(ref, color, constOpt)                                                                                               \
    inline Image<Cu##color, Backend::ref> ConvertTo##ref##_##color(constOpt Cu##color::Type *data, size_t width, size_t height, size_t linesize) \
    {                                                                                                                                            \
        return ConvertTo##ref<Cu##color>(data, width, height, linesize);                                                                         \
    }

    MakeConvertRawToRef(Ref, RGB, );
    MakeConvertRawToRef(Ref, RGBA, );

    MakeConvertRawToRef(ConstRef, RGB, const);
    MakeConvertRawToRef(ConstRef, RGBA, const);
#undef MakeConvertRawToRef
};