#pragma once

#include "Image.hpp"

namespace CuImg::Detail::WxWidgets
{
    inline void WxImageLoadFileImpl(const std::filesystem::path &path, DiscreteImageRGBA_wxWidgets &img)
    {
        const auto u8P = path.u8string();
        if (!img.GetContext().Raw().LoadFile(wxString::FromUTF8(reinterpret_cast<const char *>(u8P.c_str()), u8P.length())))
            throw CuImg_WxImageException("[wxImage::LoadFile] return false");
        if (!img.GetContext().Raw().HasAlpha())
            img.GetContext().Raw().InitAlpha();
    }

    inline void WxImageSaveFileImpl(const std::filesystem::path &path, const DiscreteImageRGBA_wxWidgets &img)
    {
        const auto u8P = path.u8string();
        if (!img.GetContext().Raw().SaveFile(wxString::FromUTF8(reinterpret_cast<const char *>(u8P.c_str()), u8P.length())))
            throw CuImg_WxImageException("[wxImage::SaveFile] return false");
    }
}