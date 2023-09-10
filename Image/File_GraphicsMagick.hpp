#pragma once

#include "Image.hpp"

#include "File/File.hpp"

namespace CuImg::Detail::GraphicsMagick
{
    inline void GraphicsMagickLoadFile(const std::filesystem::path &path, DiscreteImageRGBOpacity16_GraphicsMagick &img)
    {
        std::optional<std::string> pathBuf{};
        try
        {
            pathBuf = path.string();
        }
        catch (...)
        {
        }

        if (pathBuf)
        {
            img.GetContext().Img.read(*pathBuf);
        }
        else
        {
            Magick::Blob data{};
            uint8_t *ptr;
            size_t size;
            CuFile::ReadAllBytesAsPtr(path, &ptr, size);
            data.updateNoCopy(ptr, size);
            img.GetContext().Img.read(data);
        }
    }

    inline void GraphicsMagickSaveFile(const std::filesystem::path &path, const DiscreteImageRGBOpacity16_GraphicsMagick &img)
    {
        std::optional<std::string> pathBuf{};
        try
        {
            pathBuf = path.string();
        }
        catch (...)
        {
        }

        auto i = img.GetContext().Img;
        if (pathBuf)
        {
            i.write(*pathBuf);
        }
        else
        {
            Magick::Blob data{};
            i.write(&data, path.extension().string().substr(1));
            CuFile::WriteAllBytes(path, static_cast<const uint8_t *>(data.data()), data.length());
        }
    }
}
