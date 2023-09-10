#pragma once

#include <limits>
#include <cstdint>

#ifdef RGB
#undef RGB
#undef max
#undef min
#endif

namespace CuImg
{
    namespace Color
    {
        template <typename T>
        struct IColor
        {
            static size_t FromMemory(const void *data, T &value)
            {
                return T::FromMemory(data, value);
            }

            static size_t ToMemory(void *data, const T &value)
            {
                return T::ToMemory(data, value);
            }

            static constexpr size_t ColorSize()
            {
                return T::ColorSize();
            }

            static constexpr size_t ComponentCount()
            {
                return T::ComponentCount();
            }
        };

        template <typename T = uint8_t>
        struct RGB : IColor<RGB<T>>
        {
            using Type = T;

            T R = 0;
            T G = 0;
            T B = 0;

            RGB() = default;
            RGB(const T v) : R(v), G(v), B(v) {}

            RGB(const T r, const T g, const T b) : R(r), G(g), B(b) {}

            RGB(const T (&arr)[3]) : R(arr[0]), G(arr[1]), B(arr[2]) {}

            template <typename DstT>
            RGB<DstT> StaticCast()
            {
                return ColorRgb<DstT>(static_cast<DstT>(R), static_cast<DstT>(G), static_cast<DstT>(B));
            }

            static size_t FromMemory(const void *data, RGB &value)
            {
                const auto *ptr = static_cast<const T *>(data);
                value.R = ptr[0];
                value.G = ptr[1];
                value.B = ptr[2];
                return ColorSize();
            }

            static size_t ToMemory(void *data, const RGB &value)
            {
                auto *ptr = static_cast<T *>(data);
                ptr[0] = value.R;
                ptr[1] = value.G;
                ptr[2] = value.B;
                return ColorSize();
            }

            static constexpr size_t ComponentCount()
            {
                return 3;
            }

            static constexpr size_t ColorSize()
            {
                return sizeof(T) * ComponentCount();
            }
        };

        template <typename T = uint8_t>
        struct BGR : IColor<BGR<T>>
        {
            using Type = T;

            T B = 0;
            T G = 0;
            T R = 0;

            BGR() = default;
            BGR(const T v) : B(v), G(v), R(v) {}

            BGR(const T b, const T g, const T r) : B(b), G(g), R(r) {}

            BGR(const T (&arr)[3]) : B(arr[0]), G(arr[1]), R(arr[2]) {}

            template <typename DstT>
            BGR<DstT> StaticCast()
            {
                return ColorRgb<DstT>(static_cast<DstT>(B), static_cast<DstT>(G), static_cast<DstT>(R));
            }

            static size_t FromMemory(const void *data, BGR &value)
            {
                const auto *ptr = static_cast<const T *>(data);
                value.B = ptr[0];
                value.G = ptr[1];
                value.R = ptr[2];
                return ColorSize();
            }

            static size_t ToMemory(void *data, const BGR &value)
            {
                auto *ptr = static_cast<T *>(data);
                ptr[0] = value.B;
                ptr[1] = value.G;
                ptr[2] = value.R;
                return ColorSize();
            }

            static constexpr size_t ComponentCount()
            {
                return 3;
            }

            static constexpr size_t ColorSize()
            {
                return sizeof(T) * ComponentCount();
            }
        };

        template <typename T = uint8_t>
        struct RGBA : IColor<RGBA<T>>
        {
            using Type = T;

            T R = 0;
            T G = 0;
            T B = 0;
            T A = std::numeric_limits<T>::max();

            RGBA() = default;
            RGBA(const T v) : R(v), G(v), B(v), A(v) {}
            RGBA(const T v, const T a) : R(v), G(v), B(v), A(a) {}
            RGBA(const T r, const T g, const T b, const T a) : R(r), G(g), B(b), A(a) {}
            RGBA(const T (&arr)[4]) : R(arr[0]), G(arr[1]), B(arr[2]), A(arr[3]) {}

            template <typename DstT>
            RGBA<DstT> StaticCast()
            {
                return ColorRgba<DstT>(static_cast<DstT>(R), static_cast<DstT>(G), static_cast<DstT>(B), static_cast<DstT>(A));
            }

            static size_t FromMemory(const void *data, RGBA &value)
            {
                const auto *ptr = static_cast<const T *>(data);
                value.R = ptr[0];
                value.G = ptr[1];
                value.B = ptr[2];
                value.A = ptr[3];
                return ColorSize();
            }

            static size_t ToMemory(void *data, const RGBA &value)
            {
                auto *ptr = static_cast<T *>(data);
                ptr[0] = value.R;
                ptr[1] = value.G;
                ptr[2] = value.B;
                ptr[3] = value.A;
                return ColorSize();
            }

            static constexpr size_t ComponentCount()
            {
                return 4;
            }

            static constexpr size_t ColorSize()
            {
                return sizeof(T) * ComponentCount();
            }
        };

        template <typename T = uint8_t>
        struct RGBOpacity : IColor<RGBOpacity<T>>
        {
            using Type = T;

            T R = 0;
            T G = 0;
            T B = 0;
            T Opacity = 0;

            RGBOpacity() = default;
            RGBOpacity(const T v) : R(v), G(v), B(v), Opacity(v) {}
            RGBOpacity(const T v, const T opacity) : R(v), G(v), B(v), Opacity(opacity) {}
            RGBOpacity(const T r, const T g, const T b, const T opacity) : R(r), G(g), B(b), Opacity(opacity) {}
            RGBOpacity(const T (&arr)[4]) : R(arr[0]), G(arr[1]), B(arr[2]), Opacity(arr[3]) {}

            template <typename DstT>
            RGBOpacity<DstT> StaticCast()
            {
                return RGBOpacity<DstT>(static_cast<DstT>(R), static_cast<DstT>(G), static_cast<DstT>(B), static_cast<DstT>(Opacity));
            }

            static size_t FromMemory(const void *data, RGBOpacity &value)
            {
                const auto *ptr = static_cast<const T *>(data);
                value.R = ptr[0];
                value.G = ptr[1];
                value.B = ptr[2];
                value.Opacity = ptr[3];
                return ColorSize();
            }

            static size_t ToMemory(void *data, const RGBOpacity &value)
            {
                auto *ptr = static_cast<T *>(data);
                ptr[0] = value.R;
                ptr[1] = value.G;
                ptr[2] = value.B;
                ptr[3] = value.Opacity;
                return ColorSize();
            }

            static constexpr size_t ComponentCount()
            {
                return 4;
            }

            static constexpr size_t ColorSize()
            {
                return sizeof(T) * ComponentCount();
            }
        };
    }

    using CuRGB = Color::RGB<uint8_t>;
    using CuRGBA = Color::RGBA<uint8_t>;
    using CuBGR = Color::BGR<uint8_t>;
    using CuRGBOpacity16 = Color::RGBOpacity<uint16_t>;

    namespace Color
    {
        template <typename Src, typename Dest>
        struct Converter
        {
            void operator()(const Src &src, Dest &dest) const
            {
                dest = src;
            }
        };

        namespace Detail
        {
            template <typename Dest, typename Src>
            Dest ConvertUIntRange(const Src val)
            {
                static_assert(std::numeric_limits<Src>::min() == 0);
                static_assert(std::numeric_limits<Dest>::min() == 0);

                if constexpr (std::is_same_v<Src, Dest>)
                    return val;

                // return static_cast<Dest>(std::round(static_cast<double>(val) * std::numeric_limits<Dest>::max() / std::numeric_limits<Src>::max()));
                return static_cast<Dest>(val * std::numeric_limits<Dest>::max() / std::numeric_limits<Src>::max());
            }

            template <typename T>
            T ReverseOpacityAlpha(const T opacity)
            {
                return std::numeric_limits<T>::max() - opacity;
            }
        }

        template <>
        struct Converter<CuRGBA, CuRGBOpacity16>
        {
            void operator()(const CuRGBA &src, CuRGBOpacity16 &dest) const
            {
                dest.R = Detail::ConvertUIntRange<uint16_t>(src.R);
                dest.G = Detail::ConvertUIntRange<uint16_t>(src.G);
                dest.B = Detail::ConvertUIntRange<uint16_t>(src.B);
                dest.Opacity = Detail::ReverseOpacityAlpha(Detail::ConvertUIntRange<uint16_t>(src.A));
            }
        };

        template <>
        struct Converter<CuRGBOpacity16, CuRGBA>
        {
            void operator()(const CuRGBOpacity16 &src, CuRGBA &dest) const
            {
                dest.R = Detail::ConvertUIntRange<uint8_t>(src.R);
                dest.G = Detail::ConvertUIntRange<uint8_t>(src.G);
                dest.B = Detail::ConvertUIntRange<uint8_t>(src.B);
                dest.A = Detail::ConvertUIntRange<uint8_t>(Detail::ReverseOpacityAlpha(src.Opacity));
            }
        };

        template <>
        struct Converter<CuRGB, CuRGBA>
        {
            uint8_t Alpha = std::numeric_limits<uint8_t>::max();

            void operator()(const CuRGB &src, CuRGBA &dest) const
            {
                dest.R = src.R;
                dest.G = src.G;
                dest.B = src.B;
                dest.A = Alpha;
            }
        };

        template <>
        struct Converter<CuRGBA, CuRGB>
        {
            CuRGB BackgroundColor = CuRGB(255, 255, 255);

            void operator()(const CuRGBA &src, CuRGB &dest) const
            {
                dest.R = src.R * src.A / 255 + BackgroundColor.R * (255 - src.A) / 255;
                dest.G = src.G * src.A / 255 + BackgroundColor.G * (255 - src.A) / 255;
                dest.B = src.B * src.A / 255 + BackgroundColor.B * (255 - src.A) / 255;
            }
        };

        template <>
        struct Converter<CuRGB, CuBGR>
        {
            void operator()(const CuRGB &src, CuBGR &dest) const
            {
                dest.R = src.R;
                dest.G = src.G;
                dest.B = src.B;
            }
        };

        template <>
        struct Converter<CuRGBOpacity16, CuBGR>
        {
            CuRGB BackgroundColor = CuRGB(255, 255, 255);

            void operator()(const CuRGBOpacity16 &src, CuBGR &dest) const
            {
                CuRGBA rgba{};
                CuRGB rgb{};
                Converter<CuRGBOpacity16, CuRGBA>{}(src, rgba);
                Converter<CuRGBA, CuRGB>{BackgroundColor}(rgba, rgb);
                Converter<CuRGB, CuBGR>{}(rgb, dest);
            }
        };

        template <>
        struct Converter<CuRGBA, CuBGR>
        {
            CuRGB BackgroundColor = CuRGB(255, 255, 255);

            void operator()(const CuRGBA &src, CuBGR &dest) const
            {
                CuRGB rgb{};
                Converter<CuRGBA, CuRGB>{BackgroundColor}(src, rgb);
                Converter<CuRGB, CuBGR>{}(rgb, dest);
            }
        };

#define MakeConvertTo(c)                                                                 \
    template <typename T>                                                                \
    Cu##c ConvertTo##c(const T &src, Converter<T, Cu##c> &&conv = Converter<T, Cu##c>{}) \
    {                                                                                    \
        Cu##c ret{};                                                                     \
        conv(src, ret);                                                                  \
        return ret;                                                                      \
    }

        MakeConvertTo(RGBOpacity16);
        MakeConvertTo(RGBA);

#undef MakeConvertTo
    }
}

namespace CuColor = CuImg::Color;
