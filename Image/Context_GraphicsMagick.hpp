#pragma once

#ifdef CU_IMG_HAS_GRAPHICSMAGICK
#include "IContext.hpp"

#include <optional>

#include <Magick++.h>

#include "../Utility/Utility.hpp"

#undef min
#undef max

namespace CuImg
{
    namespace Detail::GraphicsMagick
    {
        [[maybe_unused]] static const auto MagickInit = []()
        {
            Magick::InitializeMagick(nullptr);
            return true;
        }();

        template <bool Const = true>
        struct IteratorPixelPacketPtr
        {
            using Type = const Magick::PixelPacket *;
        };
        template <>
        struct IteratorPixelPacketPtr<false>
        {
            using Type = Magick::PixelPacket *;
        };
    }

    struct GraphicsMagickContext : IDiscreteImageContext<GraphicsMagickContext, CuRGBOpacity16>
    {
        struct LoadInfo
        {

        } Info{};

        struct SaveParam
        {

        } Param{};

        template <bool Const = false, typename PixelPacketPtr = typename Detail::GraphicsMagick::IteratorPixelPacketPtr<Const>::Type>
        class iterator
        {
            struct Context
            {
                PixelPacketPtr Pixels = nullptr;
                size_t Size = 0;
                size_t Idx = Nop();

                static constexpr size_t Nop()
                {
                    return ~0ULL;
                }
            } ctx;

            std::optional<Magick::Pixels> val{};

            struct Proxy
            {
                PixelPacketPtr Ptr = nullptr;

                Proxy &operator=(const CuRGBOpacity16 &pix)
                {
                    Ptr->red = pix.R;
                    Ptr->green = pix.G;
                    Ptr->blue = pix.B;
                    Ptr->opacity = pix.Opacity;
                    return *this;
                }

                operator CuRGBOpacity16() const
                {
                    return {
                        Ptr->red,
                        Ptr->green,
                        Ptr->blue,
                        Ptr->opacity};
                }
            };

            void Clear()
            {
                if (!Const && val)
                {
                    val->sync();
                }
                val.reset();
                ctx = {};
            }

        public:
            using iterator_category = std::random_access_iterator_tag;
            using value_type = CuRGBOpacity16;
            using difference_type = std::int64_t;
            using pointer = CuRGBOpacity16 *;
            using reference = Proxy;

            iterator() {}
            iterator(Magick::Image &img)
            {
                if constexpr (!Const)
                    img.modifyImage();
                val.emplace(img);
                const auto cs = img.columns();
                const auto rs = img.rows();
                ctx.Pixels = val->get(0, 0, cs, rs);
                ctx.Size = cs * rs;
                if (cs && rs)
                    ctx.Idx = 0;
            }
            iterator(const iterator &) = delete;
            iterator(iterator &&it) noexcept
            {
                Clear();
                ctx = std::move(it.ctx);
                val = std::move(it.val);
                it.val = {};
                it.ctx = {};
            }
            ~iterator()
            {
                Clear();
            }

            iterator &operator=(const iterator &) = delete;
            iterator &operator=(iterator &&it) noexcept
            {
                Clear();
                ctx = std::move(it.ctx);
                val = std::move(it.val);
                it.val = {};
                it.ctx = {};
                return *this;
            }

            iterator &operator++()
            {
                ctx.Idx += 1;
                if (ctx.Idx >= ctx.Size)
                    ctx.Idx = Context::Nop();
                return *this;
            }

            bool operator==(const iterator &other) const { return ctx.Idx == other.ctx.Idx; }
            bool operator!=(const iterator &other) const { return !(*this == other); }

            reference operator*() const { return Proxy{ctx.Pixels + ctx.Idx}; }
        };

        Magick::Image Img{};

        void Create(const size_t width, const size_t height)
        {
            CuUtil_Assert(width < std::numeric_limits<int>::max() && height < std::numeric_limits<int>::max(), GraphicsMagickException);
            Img = Magick::Image(Magick::Geometry(static_cast<int>(width), static_cast<int>(height)), {});
        }

        [[nodiscard]] size_t Width() const { return Img.size().width(); }
        [[nodiscard]] size_t Height() const { return Img.size().height(); }
        [[nodiscard]] size_t Count() const { return Width() * Height(); }
        [[nodiscard]] bool Empty() const { return !Img.isValid(); }

        [[nodiscard]] const Magick::Image &Raw() const { return Img; }
        [[nodiscard]] Magick::Image &Raw() { return Img; }

        [[nodiscard]] const LoadInfo &GetInfo() const { return Info; }
        [[nodiscard]] LoadInfo &GetInfo() { return Info; }
        [[nodiscard]] const SaveParam &GetParam() const { return Param; }
        [[nodiscard]] SaveParam &GetParam() { return Param; }

        [[nodiscard]] iterator<false> begin() { return {Img}; }
        [[nodiscard]] iterator<true> begin() const { return {const_cast<Magick::Image &>(Img)}; }
        [[nodiscard]] iterator<false> end() { return {}; }
        [[nodiscard]] iterator<true> end() const { return {}; }

        [[nodiscard]] CuRGBOpacity16 At(const size_t row, const size_t col) const
        {
            CuUtil_Assert(col < std::min<size_t>(Width(), std::numeric_limits<int>::max()) && row < std::min<size_t>(Height(), std::numeric_limits<int>::max()), GraphicsMagickException);

            const auto *pix = GetPixels().get(static_cast<int>(col), static_cast<int>(row), 1, 1);
            return {
                pix->red,
                pix->green,
                pix->blue,
                pix->opacity};
        }

        GraphicsMagickContext &Set(const size_t row, const size_t col, const CuRGBOpacity16 &pixel)
        {
            CuUtil_Assert(col < std::min<size_t>(Width(), std::numeric_limits<int>::max()) && row < std::min<size_t>(Height(), std::numeric_limits<int>::max()), GraphicsMagickException);

            auto pixels = GetPixels();
            auto *pix = pixels.get(static_cast<int>(col), static_cast<int>(row), 1, 1);
            pix->red = pixel.R;
            pix->green = pixel.G;
            pix->blue = pixel.B;
            pix->opacity = pixel.Opacity;
            pixels.sync();
            return *this;
        }

    private:
        void EnsureModifyImage()
        {
            Img.modifyImage();
        }

        [[nodiscard]] Magick::Pixels GetPixels() const
        {
            return {const_cast<Magick::Image &>(Img)};
        }

        [[nodiscard]] Magick::Pixels GetPixels()
        {
            EnsureModifyImage();
            return {Img};
        }
    };
}
#endif