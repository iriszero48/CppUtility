#pragma once

/*
 * #define CU_IMG_HAS_DIRECTXTEX
 * #define CU_IMG_HAS_STB
 * #define CU_IMG_HAS_OPENCV
 * #define CU_IMG_HAS_QT
 * #define CU_IMG_HAS_WXWIDGETS
 * #define CU_IMG_HAS_OPENCV
 * #define CU_IMG_HAS_GRAPHICSMAGICK
 */

#include <algorithm>
#include <cassert>
#include <climits>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <sstream>

#include "Context.hpp"

namespace CuImg
{
    template <typename T = CuRGBA, Backend ImageBackend = Backend::None>
    class DiscreteImage // : IDiscreteImageContext<DiscreteImage<T, ImageBackend>, T>
    {
    public:
        using ContextType = typename DiscreteImageContextType<T, ImageBackend>::Type;
        using PixelType = T;

    protected:
        ContextType ctx{};

    public:
        DiscreteImage() = default;
        DiscreteImage(const size_t width, const size_t height)
        {
            Create(width, height);
        }

        void Create(const size_t width, const size_t height)
        {
            ctx.Create(width, height);
        }

        [[nodiscard]] size_t Width() const { return ctx.Width(); }
        [[nodiscard]] size_t Height() const { return ctx.Height(); }
        [[nodiscard]] size_t Size() const { return ctx.Size(); }
        [[nodiscard]] size_t Count() const { return ctx.Count(); }
        [[nodiscard]] bool Empty() const { return ctx.Empty(); }

        [[nodiscard]] const ContextType &GetContext() const { return ctx; }
        [[nodiscard]] ContextType &GetContext() { return ctx; }

        [[nodiscard]] decltype(auto) Raw() const { return ctx.Raw(); }
        [[nodiscard]] decltype(auto) Raw() { return ctx.Raw(); }

        [[nodiscard]] decltype(auto) GetInfo() const { return ctx.GetInfo(); }
        [[nodiscard]] decltype(auto) GetInfo() { return ctx.GetInfo(); }
        [[nodiscard]] decltype(auto) GetParam() const { return ctx.GetParam(); }
        [[nodiscard]] decltype(auto) GetParam() { return ctx.GetParam(); }

        [[nodiscard]] decltype(auto) begin() { return ctx.begin(); }
        [[nodiscard]] decltype(auto) begin() const { return ctx.begin(); }
        [[nodiscard]] decltype(auto) end() { return ctx.end(); }
        [[nodiscard]] decltype(auto) end() const { return ctx.end(); }

        [[nodiscard]] decltype(auto) At(const size_t row, const size_t col) const
        {
            return ctx.At(row, col);
        }

        decltype(auto) Set(const size_t row, const size_t col, const T &pix)
        {
            ctx.Set(row, col, pix);
            return *this;
        }
    };

    /*
     * layout:
     * Color[RGB,RGBA,...] Color ... Gap[optional] Gap ...
     * ...
     */
    template <typename T = Color::RGBA<uint8_t>, Backend ImageBackend = Backend::None>
    class Image // : IImageContext<Image<T, ImageBackend>, T>
    {
    public:
        using ContextType = typename ImageContextType<T, ImageBackend>::Type;
        using PixelType = T;

    protected:
        ContextType ctx{};

    public:
        Image() = default;
        Image(const size_t width, const size_t height)
        {
            Create(width, height);
        }

        void Create(const size_t width, const size_t height)
        {
            ctx.Create(width, height);
        }

        [[nodiscard]] decltype(auto) Data() { return ctx.Data(); }
        [[nodiscard]] decltype(auto) Data() const { return ctx.Data(); }
        [[nodiscard]] size_t Width() const { return ctx.Width(); }
        [[nodiscard]] size_t Height() const { return ctx.Height(); }
        [[nodiscard]] size_t Size() const { return ctx.Size(); }
        [[nodiscard]] size_t Count() const { return ctx.Count(); }
        [[nodiscard]] size_t Linesize() const { return ctx.Linesize(); }
        [[nodiscard]] bool Empty() const { return ctx.Empty(); }

        [[nodiscard]] const ContextType &GetContext() const { return ctx; }
        [[nodiscard]] ContextType &GetContext() { return ctx; }

        [[nodiscard]] decltype(auto) Raw() const { return ctx.Raw(); }
        [[nodiscard]] decltype(auto) Raw() { return ctx.Raw(); }

        [[nodiscard]] decltype(auto) GetInfo() const { return ctx.GetInfo(); }
        [[nodiscard]] decltype(auto) GetInfo() { return ctx.GetInfo(); }
        [[nodiscard]] decltype(auto) GetParam() const { return ctx.GetParam(); }
        [[nodiscard]] decltype(auto) GetParam() { return ctx.GetParam(); }

        [[nodiscard]] decltype(auto) begin() { return ctx.begin(); }
        [[nodiscard]] decltype(auto) begin() const { return ctx.begin(); }
        [[nodiscard]] decltype(auto) end() { return ctx.end(); }
        [[nodiscard]] decltype(auto) end() const { return ctx.end(); }

        [[nodiscard]] decltype(auto) At(const size_t row, const size_t col) const
        {
            return ctx.At(row, col);
        }

        decltype(auto) Set(const size_t row, const size_t col, const T &pix)
        {
            ctx.Set(row, col, pix);
            return *this;
        }
    };

#pragma region TypedefImage
    using ImageRGB = Image<CuRGB, Backend::None>;
    using ImageRGBA = Image<CuRGBA, Backend::None>;
    using ImageBGR = Image<CuBGR, Backend::None>;

#define MakeImage(C, B) using Image##C##_##B = Image<Cu##C, Backend::B>
#define MakeDiscreteImage(C, B) using DiscreteImage##C##_##B = DiscreteImage<Cu##C, Backend::B>

    MakeImage(RGB, Ref);
    MakeImage(RGBA, Ref);
    MakeImage(BGR, Ref);
    MakeImage(RGB, ConstRef);
    MakeImage(RGBA, ConstRef);
    MakeImage(BGR, ConstRef);

#ifdef CU_IMG_HAS_STB
    MakeImage(RGBA, STB);
    MakeImage(RGB, STB);
#endif

#ifdef CU_IMG_HAS_DIRECTXTEX
    MakeImage(RGBA, DirectXTex);
#endif

#ifdef CU_IMG_HAS_QT
    MakeImage(RGBA, QT);
    MakeImage(RGB, QT);
#endif

#ifdef CU_IMG_HAS_OPENCV
    MakeImage(RGBA, OpenCV);
    MakeImage(RGB, OpenCV);
    MakeImage(BGR, OpenCV);
#endif

#ifdef CU_IMG_HAS_WXWIDGETS
    MakeDiscreteImage(RGBA, wxWidgets);
#endif

#ifdef CU_IMG_HAS_GRAPHICSMAGICK
    MakeDiscreteImage(RGBOpacity16, GraphicsMagick);
#endif

#undef MakeImage
#undef MakeDiscreteImage
#pragma endregion TypedefImage
};
