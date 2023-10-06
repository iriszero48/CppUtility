#pragma once

#include "Image.hpp"

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "../File/File.hpp"

namespace CuImg::Detail::OCV
{
    template <typename T>
    void OpenCvLoadFile(const std::filesystem::path &path, Image<T, Backend::OpenCV> &img)
    {
        using CtxType = typename Image<T, Backend::OpenCV>::ContextType;

        CtxType ctx{};

        std::optional<std::string> pathBuf{};
        try
        {
            pathBuf = path.string();
        }
        catch (...)
        {
        }

        cv::Mat buf{};
        if (pathBuf)
        {
            buf = imread(*pathBuf, cv::IMREAD_UNCHANGED);
        }
        else
        {
            buf = cv::imdecode(CuFile::ReadAllBytes(path), cv::IMREAD_UNCHANGED);
        }
        if (buf.data == nullptr)
        {
            throw CuImg_OpenCvException(u8"the image cannot be read(because of missing file, improper permissions, unsupported or invalid format)");
        }

        if ((buf.type() & CV_MAT_DEPTH_MASK) != CV_8U)
        {
            cv::Mat conv{};
            buf.convertTo(conv, CtxType::GetCvType());
            buf = conv;
        }

        std::optional<cv::ColorConversionCodes> code{};
        if (std::is_same_v<T, CuRGB> || std::is_same_v<T, CuRGBA>)
        {
            if (T::ComponentCount() == 3 && buf.channels() == 3)
            {
                code = cv::COLOR_BGR2RGB;
            }
            else if (T::ComponentCount() == 4 && buf.channels() == 4)
            {
                code = cv::COLOR_BGRA2RGBA;
            }
            else if (T::ComponentCount() == 3 && buf.channels() == 4)
            {
                code = cv::COLOR_BGRA2RGB;
            }
            else if (T::ComponentCount() == 4 && buf.channels() == 3)
            {
                code = cv::COLOR_BGR2RGBA;
            }
            else if (T::ComponentCount() == 3 && buf.channels() == 1)
            {
                code = cv::COLOR_GRAY2RGB;
            }
            else if (T::ComponentCount() == 4 && buf.channels() == 1)
            {
                code = cv::COLOR_GRAY2RGBA;
            }
        }
        if (!code)
            throw CuImg_OpenCvException(u8"unknown color");
        cv::cvtColor(buf, ctx.Image, *code);

        img.GetContext() = std::move(ctx);
    }

    template <typename T>
    void OpenCvSaveFile(const std::filesystem::path &path, const Image<T, Backend::OpenCV> &img)
    {
        cv::Mat out{};
        const cv::Mat *ptr = &out;
        std::optional<cv::ColorConversionCodes> code{};
        if (std::is_same_v<T, CuRGB>)
            code = cv::COLOR_RGB2BGR;
        else if (std::is_same_v<T, CuRGBA>)
            code = cv::COLOR_RGBA2BGRA;

        if (code.value() != cv::COLOR_COLORCVT_MAX)
        {
            cv::cvtColor(img.GetContext().Image, out, code.value());
        }
        else
        {
            ptr = &img.GetContext().Image;
        }

        std::optional<std::string> pathBuf{};
        try
        {
            pathBuf = path.string();
        }
        catch (...)
        {
        }

        std::vector<int> params{};
        for (const auto &[k, v] : img.GetParam().Params)
        {
            params.push_back(k);
            params.push_back(v);
        }

        if (pathBuf)
        {
            if (!imwrite(*pathBuf, *ptr, params))
            {
                throw CuImg_OpenCvException(u8"the image cannot be save");
            }
        }
        else
        {
            std::string ext{};
            try
            {
                ext = path.extension().string();
            }
            catch (...)
            {
                std::rethrow_if_nested(CuImg_OpenCvException(u8"unknown extension"));
            }
            std::vector<uint8_t> data{};
            if (!imencode(ext, *ptr, data, params))
            {
                throw CuImg_OpenCvException(u8"the image cannot be save");
            }
            CuFile::WriteAllBytes(path, data.data(), data.size());
        }
    }
}