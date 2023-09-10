#pragma once

#include "IContext.hpp"
#include "Exception.hpp"

#include <vector>

namespace CuImg
{
    template <typename T>
    struct StdContext : IImageContext<StdContext<T>, T>
    {
        struct LoadInfo
        {
            size_t Gap = 0;
        } Info{};

        struct SaveParam
        {

        } Param{};

        std::vector<typename T::Type> ImageData{};
        size_t ImageWidth = 0;
        size_t ImageHeight = 0;

        void Create(const size_t width, const size_t height)
        {
            this->ImageWidth = width;
            this->ImageHeight = height;

            ImageData.resize(Linesize() * height, 0);
        }

        [[nodiscard]] const typename T::Type *Data() const { return ImageData.data(); }
        [[nodiscard]] typename T::Type *Data() { return ImageData.data(); }
        [[nodiscard]] size_t Width() const { return ImageWidth; }
        [[nodiscard]] size_t Height() const { return ImageHeight; }
        [[nodiscard]] size_t Size() const { return ImageData.size(); }
        [[nodiscard]] size_t Count() const { return Width() * Height(); }
        [[nodiscard]] size_t Linesize() const { return Width() * T::ColorSize() + GetInfo().Gap; }
        [[nodiscard]] size_t Empty() const { return ImageData.empty(); }

        [[nodiscard]] const std::vector<typename T::Type> &Raw() const { return ImageData; }
        [[nodiscard]] std::vector<typename T::Type> &Raw() { return ImageData; }

        [[nodiscard]] const LoadInfo &GetInfo() const { return Info; }
        [[nodiscard]] LoadInfo &GetInfo() { return Info; }
        [[nodiscard]] const SaveParam &GetParam() const { return Param; }
        [[nodiscard]] SaveParam &GetParam() { return Param; }
    };

    template <typename T, typename DataType = typename T::Type, typename PointerType = DataType *>
    struct RefContext : public IImageContext<RefContext<T>, T>, public IRefContext<PointerType>
    {
        struct LoadInfo
        {

        } Info{};

        struct SaveParam
        {

        } Param{};

        static void Create(const size_t, const size_t)
        {
            throw CuImg_StdException("disabled");
        }

        [[nodiscard]] const DataType *Data() const { return this->RefData; }
        [[nodiscard]] PointerType Data() { return this->RefData; }
        [[nodiscard]] size_t Width() const { return this->RefWidth; }
        [[nodiscard]] size_t Height() const { return this->RefHeight; }
        [[nodiscard]] size_t Size() const { return Linesize() * Height(); }
        [[nodiscard]] size_t Count() const { return Width() * Height(); }
        [[nodiscard]] size_t Linesize() const { return this->RefLinesize; }
        [[nodiscard]] bool Empty() const { return Width() == 0 || Height() == 0; }

        [[nodiscard]] decltype(auto) Raw() const { return Data(); }
        [[nodiscard]] decltype(auto) Raw() { return Data(); }

        [[nodiscard]] const LoadInfo &GetInfo() const { return Info; }
        [[nodiscard]] LoadInfo &GetInfo() { return Info; }
        [[nodiscard]] const SaveParam &GetParam() const { return Param; }
        [[nodiscard]] SaveParam &GetParam() { return Param; }
    };

    template <typename T, typename DataType = typename T::Type, typename PointerType = const DataType *>
    struct ConstRefContext : IImageContext<ConstRefContext<T>, T>, IRefContext<PointerType>
    {
        struct LoadInfo
        {

        } Info{};

        struct SaveParam
        {

        } Param{};

        static void Create(const size_t, const size_t)
        {
            throw CuImg_StdException("disabled");
        }

        [[nodiscard]] PointerType Data() const { return this->RefData; }
        [[nodiscard]] DataType *Data() { throw CuImg_StdException("disabled"); }
        [[nodiscard]] size_t Width() const { return this->RefWidth; }
        [[nodiscard]] size_t Height() const { return this->RefHeight; }
        [[nodiscard]] size_t Size() const { return Linesize() * Height(); }
        [[nodiscard]] size_t Count() const { return Width() * Height(); }
        [[nodiscard]] size_t Linesize() const { return this->RefLinesize; }
        [[nodiscard]] bool Empty() const { return Width() == 0 || Height() == 0; }

        [[nodiscard]] decltype(auto) Raw() const { return Data(); }
        [[nodiscard]] decltype(auto) Raw() { return Data(); }

        [[nodiscard]] const LoadInfo &GetInfo() const { return Info; }
        [[nodiscard]] LoadInfo &GetInfo() { return Info; }
        [[nodiscard]] const SaveParam &GetParam() const { return Param; }
        [[nodiscard]] SaveParam &GetParam() { return Param; }
    };
};