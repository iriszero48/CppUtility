#pragma once
#ifdef CU_IMG_HAS_WXWIDGETS

#include "IContext.hpp"

#include "wx/image.h"

#include "../Utility/Utility.hpp"

#include <string>
#include <mutex>

namespace CuImg
{
	namespace Detail::WxImage
	{
        static std::once_flag InitFlag{};

        struct Config
        {
	        bool DoNotInit = false;
        };

        template <bool C = true>
        struct Ptr
        {
            using Value = const uint8_t *;
        };

        template <>
        struct Ptr<false>
        {
            using Value = uint8_t *;
        };
	}

    struct WxImageDiscreteContext : IDiscreteImageContext<WxImageDiscreteContext, CuRGBA>
    {
        struct LoadInfo
        {

        } Info{};

        struct SaveParam
        {

        } Param{};

        wxImage Img{};

        static Detail::WxImage::Config InitConfig;

        static void Init()
        {
            std::call_once(Detail::WxImage::InitFlag, []() { if (!InitConfig.DoNotInit) wxInitAllImageHandlers(); });
        }

        WxImageDiscreteContext()
        {
            Init();
        }

        template <bool Const = false>
        class iterator
        {
            constexpr static size_t Nop()
            {
                return ~0ULL;
            }

            struct Context
            {
                typename Detail::WxImage::Ptr<Const>::Value Rgb = nullptr;
                typename Detail::WxImage::Ptr<Const>::Value A = nullptr;
                size_t Count = 0;
                size_t Idx = Nop();
            } ctx;

            struct Proxy
            {
                typename Detail::WxImage::Ptr<Const>::Value Rgb = nullptr;
                typename Detail::WxImage::Ptr<Const>::Value a = nullptr;

                Proxy &operator=(const CuRGBA &pix)
                {
                    Rgb[0] = pix.R;
                    Rgb[1] = pix.G;
                    Rgb[2] = pix.B;
                    a[0] = pix.A;
                    return *this;
                }

                operator CuRGBA() const
                {
                    return {
                        Rgb[0],
                        Rgb[1],
                        Rgb[2],
                        a[0]};
                }
            };

        public:
            using iterator_category = std::random_access_iterator_tag;
            using value_type = CuRGBOpacity16;
            using difference_type = std::int64_t;
            using pointer = const CuRGBOpacity16 *;
            using reference = Proxy;

            iterator() {}

            iterator(wxImage &img)
            {
                if (img.IsOk())
                {
                    ctx.Rgb = img.GetData();
                    ctx.A = img.GetAlpha();
                    ctx.Count = img.GetWidth() * img.GetHeight();
                    ctx.Idx = 0;
                }
            }

            iterator &operator++()
            {
                ++ctx.Idx;
                if (ctx.Idx >= ctx.Count)
                    ctx.Idx = Nop();
                return *this;
            }

            bool operator==(iterator other) const { return ctx.Idx == other.ctx.Idx; }
            bool operator!=(iterator other) const { return !(*this == other); }
            // bool operator<(iterator other) const { return idx < other.idx; }

            reference operator*() { return Proxy{ctx.Rgb + ctx.Idx * 3, ctx.A + ctx.Idx}; }
        };

        void Create(const size_t width, const size_t height)
        {
            CuAssert(width <= std::numeric_limits<int>::max() && height <= std::numeric_limits<int>::max());

            Img = {};
            Img.Create(static_cast<int>(width), static_cast<int>(height));
            Img.InitAlpha();
        }

        [[nodiscard]] size_t Width() const { return Empty() ? 0 : Img.GetWidth(); }
        [[nodiscard]] size_t Height() const { return Empty() ? 0 : Img.GetHeight(); }
        [[nodiscard]] size_t Count() const { return Empty() ? 0 : Img.GetWidth() * Img.GetHeight(); }
        [[nodiscard]] bool Empty() const { return !Img.Ok(); }

        [[nodiscard]] const wxImage &Raw() const { return Img; }
        [[nodiscard]] wxImage &Raw() { return Img; }

        [[nodiscard]] const LoadInfo &GetInfo() const { return Info; }
        [[nodiscard]] LoadInfo &GetInfo() { return Info; }
        [[nodiscard]] const SaveParam &GetParam() const { return Param; }
        [[nodiscard]] SaveParam &GetParam() { return Param; }

        [[nodiscard]] iterator<false> begin() { return {Img}; }
        [[nodiscard]] iterator<true> begin() const { return {const_cast<wxImage &>(Img)}; }
        [[nodiscard]] iterator<false> end() { return {}; }
        [[nodiscard]] iterator<true> end() const { return {}; }

        [[nodiscard]] CuRGBA At(const size_t row, const size_t col) const
        {
            CuAssert(row < Height() && col < Width());

            const auto *rgb = Img.GetData() + row * Width() * 3 * 1 + col * 3 * 1;
            return {
                rgb[0],
                rgb[1],
                rgb[2],
                Img.GetAlpha()[row * Width() * 1 * 1 + col * 1 * 1]};
        }

        decltype(auto) Set(const size_t row, const size_t col, const CuRGBA &pix)
        {
            CuAssert(row < Height() && col < Width());

            auto *rgb = Img.GetData() + row * Width() * 3 * 1 + col * 3 * 1;
            auto *a = Img.GetAlpha() + row * Width() * 1 * 1 + col * 1 * 1;
            rgb[0] = pix.R;
            rgb[1] = pix.G;
            rgb[2] = pix.B;
            a[0] = pix.A;
            return *this;
        }
    };

    Detail::WxImage::Config WxImageDiscreteContext::InitConfig = {};
}
#endif