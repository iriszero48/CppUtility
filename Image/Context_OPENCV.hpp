#pragma once

#include "IContext.hpp"

#ifdef CU_IMG_HAS_OPENCV

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

namespace CuImg
{
    namespace Detail::OCV
    {
        template <typename T>
        struct ToOpenCvFormat
        {
        };
        template <>
        struct ToOpenCvFormat<CuRGB>
        {
            static constexpr auto Format() { return CV_8UC3; }
        };
        template <>
        struct ToOpenCvFormat<CuRGBA>
        {
            static constexpr auto Format() { return CV_8UC4; }
        };
        template <>
        struct ToOpenCvFormat<CuBGR>
        {
            static constexpr auto Format() { return CV_8UC3; }
        };
    }

    template <typename T>
    struct OpenCvContext : IImageContext<OpenCvContext<T>, T>
    {
        using Type = T;

        static constexpr auto GetCvType()
        {
            return Detail::OCV::ToOpenCvFormat<T>::Format();
        }

        struct LoadInfo
        {

        } Info{};

        struct SaveParam
        {
            std::unordered_map<cv::ImwriteFlags, int> Params{};
        } Param{};

        cv::Mat Image{};

        void Create(const size_t width, const size_t height)
        {
            CuAssert(width < std::numeric_limits<int>::max() && height < std::numeric_limits<int>::max());

            Image = cv::Mat(static_cast<int>(height), static_cast<int>(width), GetCvType());
        }

        [[nodiscard]] const uint8_t *Data() const { return Image.data; }
        [[nodiscard]] uint8_t *Data() { return Image.data; }
        [[nodiscard]] size_t Width() const { return Image.cols; }
        [[nodiscard]] size_t Height() const { return Image.rows; }
        [[nodiscard]] size_t Size() const { return Linesize() * Height(); }
        [[nodiscard]] size_t Count() const { return Width() * Height(); }
        [[nodiscard]] size_t Linesize() const { return Width() * Image.elemSize(); }
        [[nodiscard]] bool Empty() const { return Image.empty(); }

        [[nodiscard]] const cv::Mat &Raw() const { return Image; }
        [[nodiscard]] cv::Mat &Raw() { return Image; }

        [[nodiscard]] const LoadInfo &GetInfo() const { return Info; }
        [[nodiscard]] LoadInfo &GetInfo() { return Info; }
        [[nodiscard]] const SaveParam &GetParam() const { return Param; }
        [[nodiscard]] SaveParam &GetParam() { return Param; }
    };
};
#endif