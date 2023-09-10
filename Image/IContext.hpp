#pragma once

#include "Color.hpp"

namespace CuImg
{
    template <typename T, typename Type>
    struct IDiscreteImageContext
    {
        void Resize(const size_t width, const size_t height) { static_cast<T *>(this)->Resize(width, height); }

        [[nodiscard]] size_t Width() const { return static_cast<const T *>(this)->Width(); }
        [[nodiscard]] size_t Height() const { return static_cast<const T *>(this)->Height(); }
        [[nodiscard]] size_t Count() const { return static_cast<const T *>(this)->Count(); }
        [[nodiscard]] bool Empty() const { return static_cast<const T *>(this)->Empty(); }

        [[nodiscard]] decltype(auto) GetInfo() const { return static_cast<const T *>(this)->GetInfo(); }
        [[nodiscard]] decltype(auto) GetInfo() { return static_cast<T *>(this)->GetInfo(); }
        [[nodiscard]] decltype(auto) GetParam() const { return static_cast<const T *>(this)->GetParam(); }
        [[nodiscard]] decltype(auto) GetParam() { return static_cast<T *>(this)->GetParam(); }

        [[nodiscard]] decltype(auto) At(const size_t row, const size_t col) const { return static_cast<const T *>(this)->At(row, col); }
        T &Set(const size_t row, const size_t col, const Type &pix) { return static_cast<T *>(this)->Set(row, col, pix); }

        [[nodiscard]] decltype(auto) begin() { return static_cast<T *>(this)->begin(); }
        [[nodiscard]] decltype(auto) begin() const { return static_cast<const T *>(this)->begin(); }
        [[nodiscard]] decltype(auto) end() { return static_cast<T *>(this)->end(); }
        [[nodiscard]] decltype(auto) end() const { return static_cast<const T *>(this)->end(); }
    };

    namespace Detail
    {
        template <typename Type, bool C = true>
        struct RawPtr
        {
            using Value = const typename Type::Type *;
        };
        template <typename Type>
        struct RawPtr<Type, false>
        {
            using Value = typename Type::Type *;
        };

        template <typename Type, bool C = true>
        struct PixPtr
        {
            using Value = const Type *;
        };
        template <typename Type>
        struct PixPtr<Type, false>
        {
            using Value = Type *;
        };

        template <typename Type, bool C = true>
        struct Ref
        {
            using Value = const Type &;
        };
        template <typename Type>
        struct Ref<Type, false>
        {
            using Value = Type &;
        };
    }

    template <typename T, typename Type>
    struct IImageContext
    {
        [[nodiscard]] size_t Width() const { return static_cast<const T *>(this)->Width(); }
        [[nodiscard]] size_t Height() const { return static_cast<const T *>(this)->Height(); }
        [[nodiscard]] size_t Count() const { return static_cast<const T *>(this)->Count(); }
        [[nodiscard]] bool Empty() const { return static_cast<const T *>(this)->Empty(); }

        [[nodiscard]] decltype(auto) Raw() const { return static_cast<const T *>(this)->Raw(); }
        [[nodiscard]] decltype(auto) Raw() { return static_cast<T *>(this)->Raw(); }

        [[nodiscard]] const typename Type::Type *Data() const { return static_cast<const T *>(this)->Data(); }
        [[nodiscard]] typename Type::Type *Data() { return static_cast<T *>(this)->Data(); }

        [[nodiscard]] size_t Size() const { return static_cast<const T *>(this)->Size(); }
        [[nodiscard]] size_t Linesize() const { return static_cast<const T *>(this)->Linesize(); }

        [[nodiscard]] Type At(const size_t row, const size_t col) const
        {
            const auto *p = Data() + (row * Linesize() + col * Type::ColorSize());
            Type res{};
            Type::FromMemory(p, res);
            return res;
        }

        T &Set(const size_t row, const size_t col, const Type &pix)
        {
            auto *p = Data() + (row * Linesize() + col * Type::ColorSize());
            Type::ToMemory(p, pix);
            return *static_cast<T *>(this);
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
                typename Detail::RawPtr<Type, Const>::Value Ptr = nullptr;
                size_t Width = 0;
                size_t Height = 0;
                size_t Linesize = 0;
                size_t CIdx = Nop();
                size_t RIdx = Nop();
            } ctx;

        public:
            using iterator_category = std::random_access_iterator_tag;
            using value_type = Type;
            using difference_type = std::int64_t;
            using pointer = typename Detail::RawPtr<Type, Const>::Value;
            using reference = typename Detail::Ref<Type, Const>::Value;

            iterator() {}

            iterator(
                typename Detail::RawPtr<Type, Const>::Value ptr,
                const size_t width,
                const size_t height,
                const size_t linesize)
            {
                if (ptr)
                {
                    ctx.Ptr = ptr;
                    ctx.Width = width;
                    ctx.Height = height;
                    ctx.Linesize = linesize;
                    ctx.CIdx = 0;
                    ctx.RIdx = 0;
                }
            }

            iterator &operator++()
            {
                ++ctx.CIdx;

                if (ctx.CIdx >= ctx.Width)
                {
                    ctx.CIdx = 0;
                    ++ctx.RIdx;
                }

                if (ctx.RIdx >= ctx.Height)
                {
                    ctx.RIdx = Nop();
                    ctx.CIdx = Nop();
                }

                return *this;
            }

            bool operator==(iterator other) const { return ctx.RIdx == other.ctx.RIdx && ctx.CIdx == other.ctx.CIdx; }
            bool operator!=(iterator other) const { return !(*this == other); }
            // bool operator<(iterator other) const { return idx < other.idx; }

            reference operator*()
            {
                return *(typename Detail::PixPtr<Type, Const>::Value)(ctx.Ptr + ctx.Linesize * ctx.RIdx + Type::ColorSize() * ctx.CIdx);
            }
        };

        [[nodiscard]] iterator<false> begin()
        {
            if (Empty())
                return {};

            return {
                Data(),
                Width(),
                Height(),
                Linesize()};
        }
        [[nodiscard]] iterator<true> begin() const
        {
            if (Empty())
                return {};

            return {
                Data(),
                Width(),
                Height(),
                Linesize()};
        }
        [[nodiscard]] iterator<false> end() { return {}; }
        [[nodiscard]] iterator<true> end() const { return {}; }
    };

    template <typename Pointer>
    struct IRefContext
    {
        Pointer RefData = nullptr;
        size_t RefWidth = 0;
        size_t RefHeight = 0;
        size_t RefLinesize = 0;

        void SetSource(Pointer data, const size_t width, const size_t height, const size_t linesize)
        {
            RefData = data;
            RefWidth = width;
            RefHeight = height;
            RefLinesize = linesize;
        }
    };
};