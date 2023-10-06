#pragma once

#include <sstream>
#include <stdexcept>
#include <string>
#include <map>
#include <unordered_map>
#include <array>
#include <optional>

#include <stdio.h>

#include "../Utility/Utility.hpp"
#include "../Exception/Except.hpp"

namespace CuEnum
{
    namespace Detail
    {
        template <typename... Args>
        std::string Combine(Args &&...args)
        {
            std::string buf{};
            (buf.append(args), ...);
            return buf;
        }

        template <typename T, size_t S>
        constexpr const char *GetFilename(const T (&str)[S], size_t i = S - 1)
        {
            for (; i > 0; --i)
                if (str[i] == '/' || str[i] == '\\')
                    return &str[i + 1];
            return str;
        }

        template <size_t S>
        constexpr std::size_t GetItemCount(const char (&str)[S])
        {
            std::size_t count = 0;
            for (std::size_t i = 0; i < S; ++i)
                if (str[i] == ',')
                    ++count;
            return count + 1;
        }
    }

    namespace Impl
    {
        template <typename EnumType>
        struct EnumValues
        {
        };

        template <typename EnumType>
        struct EnumStrings
        {
        };

        template <typename EnumType>
        struct EnumFromString
        {
        };

        template <typename EnumType>
        struct EnumToString
        {
        };
    }

    class Exception : public std::runtime_error
    {
    public:
        using std::runtime_error::runtime_error;
    };

    template <typename T>
    constexpr std::string_view ToString(const T in)
    {
        return Impl::EnumToString<T>{}(in);
    }

    template <typename T>
    constexpr std::optional<T> FromString(const std::string_view &str)
    {
        return Impl::EnumFromString<T>{}(str);
    }

    template <typename T>
    T TryFromString(const std::string_view &str)
    {
        const auto v = FromString<T>(str);
        if (!v)
        {
            throw Exception("value is not one of the named constants defined for the enumeration");
        }

        return *v;
    }

    template <typename T>
    constexpr decltype(auto) Values()
    {
        return Impl::EnumValues<T>{}();
    }

    template <typename T>
    constexpr decltype(auto) Strings()
    {
        return Impl::EnumStrings<T>{}();
    }

    template <typename T, size_t S>
    constexpr size_t GetMetaTableSize(const std::array<T, S> &str)
    {
        size_t count = 0;
        for (size_t i = 0; i != S - 1; ++i)
        {
            if (str[i] == ',')
                count++;
        }
        if (str[S - 1 - 1] == ',')
            count--;
        return count + 1;
    }

    template <typename T>
    struct MetaRecord
    {
        T Value;
        size_t Begin;
        size_t End;
    };

    template <size_t Count, typename EnumT, typename T, size_t S>
    constexpr auto GetMetaTable(const std::array<T, S> &str)
    {
        constexpr int64_t strBeg = 0;
        constexpr int64_t strEnd = S - 1;

        using ValueType = std::underlying_type_t<EnumT>;

        ValueType nextVal = 0;
        size_t resultIdx = 0;

        std::array<MetaRecord<ValueType>, Count> result{};

        int64_t rawItemBeg = strBeg;
        int64_t rawItemEnd = strBeg - 1;
        for (int64_t i = strBeg; i != S; ++i)
        {
            if (str[i] == ',' || i == strEnd)
            {
                rawItemBeg = rawItemEnd + 1;
                rawItemEnd = i;

                size_t itemBeg = rawItemBeg;
                size_t itemEnd = rawItemEnd;
                const auto itemRng = CuUtil::String::Trim(str, itemBeg, itemEnd, ' ');
                itemBeg = itemRng.Begin;
                itemEnd = itemRng.End;

                int64_t eqPos = -1;
                for (size_t j = itemBeg; j != itemEnd; ++j)
                {
                    if (str[j] == '=')
                    {
                        eqPos = static_cast<int64_t>(j);
                        break;
                    }
                }

                if (eqPos != -1)
                {
                    const auto kBegin = itemBeg;
                    const auto kEnd = CuUtil::String::TrimEnd(str, itemBeg, eqPos, ' ');

                    const auto vBegin = CuUtil::String::TrimBegin(str, eqPos + 1, itemEnd, ' ');
                    const auto vEnd = itemEnd;
                    if (vEnd - vBegin > 2 && str[vBegin] == '0' && (str[vBegin + 1] == 'x' || str[vBegin + 1] == 'X'))
                    {
                        nextVal = CuUtil::Convert::ToIntegral<ValueType>(str, vBegin, vEnd, 16);
                    }
                    else
                    {
                        nextVal = CuUtil::Convert::ToIntegral<ValueType>(str, vBegin, vEnd);
                    }
                    result[resultIdx++] = MetaRecord<ValueType>{nextVal++, (size_t)kBegin, (size_t)kEnd};
                }
                else
                {
                    result[resultIdx++] = MetaRecord<ValueType>{nextVal++, (size_t)itemBeg, (size_t)itemEnd};
                }
            }
        }

        return result;
    }
}

#define CuEnum_MakeEnumDefWithType(enumName, type, ...)                                                              \
    enum class enumName : type                                                                                       \
    {                                                                                                                \
        __VA_ARGS__                                                                                                  \
    };                                                                                                               \
    static constexpr auto CuEnum_##enumName##_RawString = CuUtil::String::ToBuffer(#__VA_ARGS__);                    \
    static constexpr auto CuEnum_##enumName##_TrimStringLength =                                                     \
        CuUtil::String::TrimLength(CuEnum_##enumName##_RawString, 0, CuEnum_##enumName##_RawString.size() - 1, ' '); \
    static constexpr auto CuEnum_##enumName##_TrimString =                                                           \
        CuUtil::String::Trim<CuEnum_##enumName##_TrimStringLength>(                                                  \
            CuEnum_##enumName##_RawString,                                                                           \
            0, CuEnum_##enumName##_RawString.size() - 1, ' ');                                                       \
    static constexpr auto CuEnum_##enumName##_MetaTableSize =                                                        \
        CuEnum::GetMetaTableSize(CuEnum_##enumName##_TrimString);                                                    \
    static constexpr auto CuEnum_##enumName##_MetaTable =                                                            \
        CuEnum::GetMetaTable<CuEnum_##enumName##_MetaTableSize, enumName>(CuEnum_##enumName##_TrimString)

#define CuEnum_MakeEnumDef(enumName, ...) CuEnum_MakeEnumDefWithType(enumName, int, __VA_ARGS__)

#define CuEnum_MakeEnumSpec(namespaceName, enumName)                                                                       \
    template <>                                                                                                            \
    struct CuEnum::Impl::EnumToString<namespaceName::enumName>                                                             \
    {                                                                                                                      \
        constexpr std::string_view operator()(const namespaceName::enumName val) const                                     \
        {                                                                                                                  \
            for (const auto &[value, begin, end] : namespaceName::CuEnum_##enumName##_MetaTable)                           \
            {                                                                                                              \
                if (value == static_cast<std::underlying_type_t<namespaceName::enumName>>(val))                            \
                {                                                                                                          \
                    return {namespaceName::CuEnum_##enumName##_TrimString.data() + begin, end - begin};                    \
                }                                                                                                          \
            }                                                                                                              \
                                                                                                                           \
            return {};                                                                                                     \
        }                                                                                                                  \
    };                                                                                                                     \
                                                                                                                           \
    template <>                                                                                                            \
    struct CuEnum::Impl::EnumFromString<namespaceName::enumName>                                                           \
    {                                                                                                                      \
        constexpr std::optional<namespaceName::enumName> operator()(const std::string_view &str) const                     \
        {                                                                                                                  \
            for (const auto &[value, begin, end] : namespaceName::CuEnum_##enumName##_MetaTable)                           \
            {                                                                                                              \
                if (const std::string_view sv{namespaceName::CuEnum_##enumName##_TrimString.data() + begin, end - begin};  \
                    sv == str)                                                                                             \
                {                                                                                                          \
                    return static_cast<namespaceName::enumName>(value);                                                    \
                }                                                                                                          \
            }                                                                                                              \
                                                                                                                           \
            return {};                                                                                                     \
        }                                                                                                                  \
    };                                                                                                                     \
                                                                                                                           \
    template <>                                                                                                            \
    struct CuEnum::Impl::EnumValues<namespaceName::enumName>                                                               \
    {                                                                                                                      \
        constexpr std::array<namespaceName::enumName, namespaceName::CuEnum_##enumName##_MetaTableSize> operator()() const \
        {                                                                                                                  \
            std::array<namespaceName::enumName, namespaceName::CuEnum_##enumName##_MetaTableSize> buf{};                   \
            for (size_t i = 0; i < namespaceName::CuEnum_##enumName##_MetaTableSize; ++i)                                  \
            {                                                                                                              \
                buf[i] = static_cast<namespaceName::enumName>(namespaceName::CuEnum_##enumName##_MetaTable[i].Value);      \
            }                                                                                                              \
            return buf;                                                                                                    \
        }                                                                                                                  \
    };                                                                                                                     \
                                                                                                                           \
    template <>                                                                                                            \
    struct CuEnum::Impl::EnumStrings<namespaceName::enumName>                                                              \
    {                                                                                                                      \
        constexpr std::array<std::string_view, namespaceName::CuEnum_##enumName##_MetaTableSize> operator()() const        \
        {                                                                                                                  \
            std::array<std::string_view, namespaceName::CuEnum_##enumName##_MetaTableSize> buf{};                          \
            for (size_t i = 0; i < namespaceName::CuEnum_##enumName##_MetaTableSize; ++i)                                  \
            {                                                                                                              \
                const auto &[_, begin, end] = namespaceName::CuEnum_##enumName##_MetaTable[i];                             \
                buf[i] = std::string_view(namespaceName::CuEnum_##enumName##_TrimString.data() + begin, end - begin);      \
            }                                                                                                              \
            return buf;                                                                                                    \
        }                                                                                                                  \
    }

#define CuEnum_MakeEnumWithType(enumName, type, ...)         \
    CuEnum_MakeEnumDefWithType(enumName, type, __VA_ARGS__); \
    CuEnum_MakeEnumSpec(, enumName)

#define CuEnum_MakeEnum(enumName, ...) CuEnum_MakeEnumWithType(enumName, int, __VA_ARGS__)
