#pragma once

#define STBI_WINDOWS_UTF8
#define STBIW_WINDOWS_UTF8
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <stb_image_write.h>

#include "Image.hpp"
#include "File_Utility.hpp"

namespace CuImg::Detail::STB
{
    template <typename T>
    void StbLoadFileImpl(const std::filesystem::path &path, Image<T, Backend::STB> &img)
    {
        using CtxType = typename Image<T, Backend::STB>::ContextType;
        CtxType ctx{};
        int w = 0;
        int h = 0;
        int comp = 0;
        ctx.ImageData = stbi_load(reinterpret_cast<const char *>(path.u8string().c_str()), &w, &h, &comp, static_cast<int>(T::ComponentCount()));
        if (ctx.ImageData == nullptr)
            throw CuImg_StbException("[stbi_load] invalid data: ", stbi_failure_reason());

        ctx.ImageWidth = w;
        ctx.ImageHeight = h;
        ctx.GetInfo().ChannelsInFile = comp;
        img.GetContext() = std::move(ctx);
    }

    template <typename T>
    void StbSaveFileImpl(const std::filesystem::path &path, const Image<T, Backend::STB> &img)
    {
        const auto &param = img.GetParam();
        if (param.JpgQuality < 1 || param.JpgQuality > 100)
            throw CuImg_StbException("[Param] assert(0 < JpgQuality <= 100)");

        const auto ext = CuStr::ToLower(path.extension().native());

        const auto f = CuStr::ToDirtyUtf8String(path.u8string());
        const int w = img.Width();
        const int h = img.Height();
        const auto *d = img.Data();
        constexpr auto c = T::ComponentCount();
        int ret;
        if (IsPNG(ext))
            ret = stbi_write_png(f.c_str(), w, h, c, d, w * T::ColorSize());
        else if (IsJPEG(ext))
            ret = stbi_write_jpg(f.c_str(), w, h, c, d, param.JpgQuality);
        else if (IsBMP(ext))
            ret = stbi_write_bmp(f.c_str(), w, h, c, d);
        else if (IsTGA(ext))
            ret = stbi_write_tga(f.c_str(), w, h, c, d);
        else
            throw CuImg_StbException("Unsupported Image Format: ", ext);

        if (!ret)
            throw CuImg_StbException("[stbi_write_*] ret ", ret);

        if (!exists(path))
            throw CuImg_StbException("save failed");
    }
};