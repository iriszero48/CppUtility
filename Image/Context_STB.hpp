#pragma once

#include "IContext.hpp"

#include "../Utility/Utility.hpp"
#include "../Assert/Assert.hpp"

#ifdef CU_IMG_HAS_STB
#define STB_IMAGE_IMPLEMENTATION

#include <stb_image.h>

namespace CuImg
{
    template <typename T>
    struct StbContext : IImageContext<StbContext<T>, T>
    {
        using ChanType = typename T::Type;
        struct LoadInfo
        {
            int ChannelsInFile;
        } Info{};

        struct SaveParam
        {
            int JpgQuality = 100;
        } Param{};

        ChanType *ImageData = nullptr;
        size_t ImageWidth = 0;
        size_t ImageHeight = 0;

        StbContext() {}

        StbContext(const StbContext &ctx)
        {
            Copy(ctx);
        }

        StbContext(StbContext &&ctx) noexcept
        {
            Move(std::move(ctx));
        }

        StbContext &operator=(const StbContext &ctx)
        {
            Copy(ctx);
            return *this;
        }

        StbContext &operator=(StbContext &&ctx) noexcept
        {
            Move(std::move(ctx));
            return *this;
        }

        ~StbContext()
        {
            Reset();
        }

        void Create(const size_t width, const size_t height)
        {
            CuAssert(width < std::numeric_limits<int>::max() && height < std::numeric_limits<int>::max());

            this->ImageWidth = width;
            this->ImageHeight = height;

            if (ImageData)
                STBI_FREE(ImageData);

            ImageData = static_cast<ChanType *>(STBI_MALLOC(Size()));
        }

        [[nodiscard]] const ChanType *Data() const { return ImageData; }
        [[nodiscard]] ChanType *Data() { return ImageData; }
        [[nodiscard]] size_t Width() const { return ImageWidth; }
        [[nodiscard]] size_t Height() const { return ImageHeight; }
        [[nodiscard]] size_t Size() const { return Linesize() * ImageHeight; }
        [[nodiscard]] size_t Count() const { return Width() * Height(); }
        [[nodiscard]] size_t Linesize() const { return Width() * T::ColorSize(); }
        [[nodiscard]] bool Empty() const { return ImageData == nullptr; }

        [[nodiscard]] const ChanType *Raw() const { return ImageData; }
        [[nodiscard]] ChanType *Raw() { return ImageData; }

        [[nodiscard]] const LoadInfo &GetInfo() const { return Info; }
        [[nodiscard]] LoadInfo &GetInfo() { return Info; }
        [[nodiscard]] const SaveParam &GetParam() const { return Param; }
        [[nodiscard]] SaveParam &GetParam() { return Param; }

    private:
        void Reset()
        {
            Info = {};
            Param = {};

            ImageWidth = 0;
            ImageHeight = 0;
            if (ImageData)
            {
                STBI_FREE(ImageData);
                ImageData = nullptr;
            }
        }

        void Copy(const StbContext &ctx)
        {
            Info = ctx.Info;
            Param = ctx.Param;

            Create(ctx.Width(), ctx.Height());
            std::copy_n(ctx.Data(), Size(), Data());
        }

        void Move(StbContext &&ctx)
        {
            Info = ctx.Info;
            Param = ctx.Param;

            ImageWidth = ctx.Width();
            ImageHeight = ctx.Height();
            ImageData = ctx.Data();
            ctx.ImageData = nullptr;
            ctx.Reset();
        }
    };
};
#endif