#pragma once

#include <algorithm>
#include <charconv>
#include <filesystem>
#include <sstream>
#include <string>
#include <vector>
#include <string_view>

#if defined(__cpp_lib_char8_t)
#define __U8stringUseChar8
#endif

namespace CuStr
{
    constexpr int Version[]{1, 0, 0, 0};

    template <typename T>
    struct IsChar : std::integral_constant<bool, std::is_same_v<T, std::string::value_type> || std::is_same_v<
	                                           T, std::wstring::value_type> ||
                                           std::is_same_v<T, decltype(std::filesystem::path{}.u8string())::value_type>
                                           ||
                                           std::is_same_v<T, std::u16string::value_type> || std::is_same_v<
	                                           T, std::u32string::value_type>>
    {
    };

    template <typename T>
    inline constexpr auto IsCharV = IsChar<T>::value;

    namespace _Detail
    {
        template <class, class = void>
        struct HasValueType : std::false_type
        {
        };

        template <class T>
        struct HasValueType<T, std::void_t<typename T::value_type>> : std::true_type
        {
        };

        template <class T, typename ToMatch>
        struct IsCString : std::integral_constant<bool, std::is_same<ToMatch const *, typename std::decay<T>::type>::value ||
                                                            std::is_same<ToMatch *, typename std::decay<T>::type>::value>
        {
        };

        template <typename T>
        [[nodiscard]] std::filesystem::path ToStringImpl(const T &t)
        {
            if constexpr (std::is_same_v<T, std::filesystem::path>)
            {
                return t;
            }
            else if constexpr (std::is_same_v<T, std::string::value_type> || std::is_same_v<T, std::wstring::value_type> ||
                               std::is_same_v<T, decltype(std::filesystem::path{}.u8string())::value_type> ||
                               std::is_same_v<T, std::u16string::value_type> || std::is_same_v<T, std::u32string::value_type>)
            {
                return std::basic_string<T>(1, t);
            }
            else if constexpr (std::is_same_v<T, bool>)
            {
                return t ? "true" : "false";
            }
            else if constexpr (std::is_integral_v<T>)
            {
                constexpr auto bufSiz = 65;
                char buf[bufSiz]{0};
                if (const auto [p, e] = std::to_chars(buf, buf + bufSiz, t); e != std::errc{})
                    throw std::runtime_error("ToStringImpl error: invalid literal: " + std::string(p));
                return buf;
            }
            else if constexpr (std::is_floating_point_v<T>)
            {
                std::ostringstream buf;
                buf << t;
                return buf.str();
            }
            else if constexpr (IsCString<T, char>::value || IsCString<T, wchar_t>::value ||
                               IsCString<T, decltype(std::filesystem::path{}.u8string())::value_type>::value ||
                               IsCString<T, char16_t>::value || IsCString<T, char32_t>::value)
            {
                return t;
            }
            else if constexpr (HasValueType<T>::value)
            {
                if constexpr ((std::is_base_of_v<std::basic_string<typename T::value_type>, T> ||
                               std::is_base_of_v<std::basic_string_view<typename T::value_type>, T>))
                {
                    return t;
                }
                else
                {
                    std::u32string str = U"{";
                    auto i = t.begin();
                    auto end = t.end();
                    if (end - i == 0)
                        return U"{}";
                    for (; i < end - 1; ++i)
                    {
                        str.append(ToStringImpl(*i).u32string());
                        str.append(U", ");
                    }
                    str.append(ToStringImpl(*i).u32string());
                    str.append(U"}");
                    return str;
                }
            }
            else
            {
                std::ostringstream buf;
                buf << t;
                return buf.str();
            }
        }

        template <typename It>
        [[nodiscard]] std::filesystem::path ToStringImpl(It begin, It end)
        {
            const auto size = end - begin;
            if (size == 0)
                return "{}";

            std::u32string str = U"{";
            for (auto i = begin; i != end; ++i)
            {
                str.append(ToStringImpl(*i).u32string());
                str.append(U", ");
            }

            str.erase(str.length() - 2);
            str.append(U"}");
            return str;
        }

        template <typename T, size_t S>
        [[nodiscard]] std::filesystem::path ToStringImpl(const std::array<T, S> &t)
        {
            return ToStringImpl(t.begin(), t.end());
        }

        template <typename T>
        struct GetStrFunc
        {
        };

        template <>
        struct GetStrFunc<std::string>
        {
            [[nodiscard]] decltype(auto) operator()(const std::filesystem::path &str) const
            {
                return str.string();
            }
        };

        template <>
        struct GetStrFunc<std::wstring>
        {
            [[nodiscard]] decltype(auto) operator()(const std::filesystem::path &str) const
            {
                return str.wstring();
            }
        };
#ifdef __U8stringUseChar8
        template <>
        struct GetStrFunc<std::u8string>
        {
            [[nodiscard]] decltype(auto) operator()(const std::filesystem::path &str) const
            {
                return str.u8string();
            }
        };
#endif
        template <>
        struct GetStrFunc<std::u16string>
        {
            [[nodiscard]] decltype(auto) operator()(const std::filesystem::path &str) const
            {
                return str.u16string();
            }
        };

        template <>
        struct GetStrFunc<std::u32string>
        {
            [[nodiscard]] decltype(auto) operator()(const std::filesystem::path &str) const
            {
                return str.u32string();
            }
        };

        template <typename CharType, typename T>
        [[nodiscard]] constexpr bool IsTypeString()
        {
            constexpr auto subcheck = _Detail::HasValueType<T>::value && (std::is_base_of_v<std::basic_string<CharType>, T> ||
                                                                          std::is_base_of_v<std::basic_string_view<CharType>, T>);
            return subcheck || _Detail::IsCString<T, CharType>::value;
        }

#ifdef __U8stringUseChar8
#define __Suit2(macro, func)          \
    macro(std::string, func);         \
    macro(std::wstring, func##W);     \
    macro(std::u8string, func##U8);   \
    macro(std::u16string, func##U16); \
    macro(std::u32string, func##U32)

#define __Suit3(macro, src, func)          \
    macro(src, std::string, func);         \
    macro(src, std::wstring, func##W);     \
    macro(src, std::u8string, func##U8);   \
    macro(src, std::u16string, func##U16); \
    macro(src, std::u32string, func##U32)
#else
#define __Suit2(macro, func)                               \
    macro(std::string, func) macro(std::wstring, func##W); \
    macro(std::u16string, func##U16) macro(std::u32string, func##U32);

#define __Suit3(macro, src, func)                                    \
    macro(src, std::string, func) macro(src, std::wstring, func##W); \
    macro(src, std::u16string, func##U16);                           \
    macro(src, std::u32string, func##U32);
#endif

#define __Args1(src, type, func)            \
    template <typename Str>                 \
    [[nodiscard]] type func(const Str &str) \
    {                                       \
        return src<type>(str);              \
    };
    } // namespace _Detail

    template <typename T>
    [[nodiscard]] constexpr bool IsCharString() { return _Detail::IsTypeString<char, T>(); }
    template <typename T>
    [[nodiscard]] constexpr bool IsWCharString() { return _Detail::IsTypeString<wchar_t, T>(); }
    template <typename T>
    [[nodiscard]] constexpr bool IsChar16String() { return _Detail::IsTypeString<char16_t, T>(); }
    template <typename T>
    [[nodiscard]] constexpr bool IsChar32String() { return _Detail::IsTypeString<char32_t, T>(); }
#ifdef __U8stringUseChar8
    template <typename T>
    [[nodiscard]] constexpr bool IsChar8String()
    {
        return _Detail::IsTypeString<char8_t, T>();
    }
#endif

    template <typename T>
    [[nodiscard]] constexpr bool IsString()
    {
        return IsCharString<T>() || IsWCharString<T>() || IsChar16String<T>() || IsChar32String<T>()
#ifdef __U8stringUseChar8
               || IsChar8String<T>()
#endif
            ;
    }

#pragma region UpperLower

    template <typename Str>
    void Upper(Str &string)
    {
        std::transform(string.begin(), string.end(), string.begin(), static_cast<int (*)(int)>(std::toupper));
    }

    template <typename Ret, typename Str>
    [[nodiscard]] Ret ToUpperAs(const Str &str)
    {
        using BufType = std::basic_string<typename Str::value_type>;
        BufType buf(str);
        Upper(buf);
        if constexpr (std::is_same_v<Ret, BufType>)
            return buf;
        return _Detail::GetStrFunc<Ret>{}(buf);
    }

    __Suit3(__Args1, ToUpperAs, ToUpper);

    template <typename Str>
    void Lower(Str &string)
    {
        std::transform(string.begin(), string.end(), string.begin(), static_cast<int (*)(int)>(std::tolower));
    }

    template <typename Ret, typename Str>
    [[nodiscard]] Ret ToLowerAs(const Str &str)
    {
        using BufType = std::basic_string<typename Str::value_type>;
        BufType buf(str);
        Lower(buf);
        if constexpr (std::is_same_v<Ret, BufType>)
            return buf;
        return _Detail::GetStrFunc<Ret>{}(buf);
    }

    __Suit3(__Args1, ToLowerAs, ToLower)
#pragma endregion UpperLower

#pragma region Pad
#define __PadImpl(src, type, funcName)                                                                         \
    template <typename Str>                                                                                    \
    [[nodiscard]] type funcName(const Str &str, const std::uint32_t width, const typename Str::value_type pad) \
    {                                                                                                          \
        return src<type>(str, width, pad);                                                                     \
    };

        template <typename Str>
        void PadLeftTo(Str &str, const std::uint32_t width, const typename Str::value_type pad)
    {
        std::int64_t n = width - str.length();
        if (n <= 0)
            return;
        str.insert(str.begin(), n, pad);
    }

    template <typename Ret, typename Str>
    [[nodiscard]] Ret PadLeftAs(const Str &str, const std::uint32_t width, const typename Str::value_type pad)
    {
        auto buf = str;
        PadLeftTo(buf, width, pad);
        if constexpr (std::is_same_v<Ret, Str>)
            return buf;
        return _Detail::GetStrFunc<Ret>{}(buf);
    }

    __Suit3(__PadImpl, PadLeftAs, PadLeft);

    template <typename Str>
    void PadRightTo(Str &str, const std::uint32_t width, const typename Str::value_type pad)
    {
        std::int64_t n = width - str.length();
        if (n <= 0)
            return;
        str.append(n, pad);
    }

    template <typename Ret, typename Str>
    [[nodiscard]] Ret PadRightAs(const Str &str, const std::uint32_t width, const typename Str::value_type pad)
    {
        auto buf = str;
        PadRightTo(buf, width, pad);
        if constexpr (std::is_same_v<Ret, Str>)
            return buf;
        return _Detail::GetStrFunc<Ret>{}(buf);
    }

    __Suit3(__PadImpl, PadRightAs, PadRight);
#pragma endregion Pad

#pragma region Appends
    template <typename Str, typename... Args>
    void AppendsTo(Str &str, Args &&...args)
    {
        (str.append(args), ...);
    }

    template <typename Ret, typename... Args>
    [[nodiscard]] Ret AppendsAs(Args &&...args)
    {
        Ret buf{};
        AppendsTo(buf, std::forward<Args>(args)...);
        return buf;
    }

#define __AppendsImpl(type, func)                            \
    template <typename... Args>                              \
    [[nodiscard]] type func(Args &&...args)                  \
    {                                                        \
        return AppendsAs<type>(std::forward<Args>(args)...); \
    }

    __Suit2(__AppendsImpl, Appends);
#pragma endregion

#pragma region Combine

    template <typename Ret, typename... Args>
    [[nodiscard]] Ret CombineAs(Args &&...args)
    {
        std::basic_ostringstream<typename Ret::value_type> buf{};
        (buf << ... << args);
        return buf.str();
    }

#define __CombineImpl(type, func)                            \
    template <typename... Args>                              \
    [[nodiscard]] type func(Args &&...args)                  \
    {                                                        \
        return CombineAs<type>(std::forward<Args>(args)...); \
    }

    __CombineImpl(std::string, Combine);
    __CombineImpl(std::wstring, CombineW);
#pragma endregion Combine

#pragma region FromStream

    template <typename T, typename... Args>
    void FromStreamTo(std::string &str, const T &toStream, Args &&...fmt)
    {
        std::ostringstream buf{};
        (buf << ... << fmt) << toStream;
        str.append(buf.str());
    }

    template <typename Ret, typename T, typename... Args>
    [[nodiscard]] Ret FromStreamAs(const T &toStream, Args &&...fmt)
    {
        std::string buf{};
        FromStreamTo(buf, toStream, std::forward<Args>(fmt)...);
        if constexpr (std::is_same_v<Ret, std::string>)
            return buf;
        return _Detail::GetStrFunc<Ret>{}(buf);
    }

#define __FromStreamImpl(type, func)                                     \
    template <typename T, typename... Args>                              \
    [[nodiscard]] type func(const T &toStream, Args &&...fmt)            \
    {                                                                    \
        return FromStreamAs<type>(toStream, std::forward<Args>(fmt)...); \
    }

    __Suit2(__FromStreamImpl, FromStream);
#pragma endregion FromStream

#pragma region ToString

    template <typename Str, typename T>
    [[nodiscard]] Str ToStringAs(const T &t)
    {
        return _Detail::GetStrFunc<Str>()(_Detail::ToStringImpl(t));
    }

    template <typename T>
    [[nodiscard]] std::string ToString(const T &t)
    {
        return ToStringAs<std::string>(t);
    }

    template <typename T>
    [[nodiscard]] std::wstring ToWString(const T &t)
    {
        return ToStringAs<std::wstring>(t);
    }
#ifdef __U8stringUseChar8
    template <typename T>
    [[nodiscard]] std::u8string ToU8String(const T &t)
    {
        return ToStringAs<std::u8string>(t);
    }
#endif
    template <typename T>
    [[nodiscard]] std::u16string ToU16String(const T &t)
    {
        return ToStringAs<std::u16string>(t);
    }

    template <typename T>
    [[nodiscard]] std::u32string ToU32String(const T &t)
    {
        return ToStringAs<std::u32string>(t);
    }

#pragma endregion ToString

#pragma region Join

    template <typename It, typename Str, typename Out>
    void JoinTo(Out &str, It beg, It end, const Str &seq)
    {
        auto i = beg;
        if (end - beg == 0)
            return;
        for (; i < end - 1; ++i)
        {
            str.append(ToStringAs<Out>(*i));
            str.append(ToStringAs<Out>(seq));
        }
        str.append(ToStringAs<Out>(*i));
    }

    template <typename Ret, typename It, typename Str>
    [[nodiscard]] Ret JoinAs(It beg, It end, const Str &seq)
    {
        Ret buf{};
        JoinTo(buf, beg, end, seq);
        return buf;
    }

#define __JoinImpl(type, func)                              \
    template <typename It, typename Str>                    \
    [[nodiscard]] type func(It beg, It end, const Str &seq) \
    {                                                       \
        return JoinAs<type>(beg, end, seq);                 \
    }

    __Suit2(__JoinImpl, Join);
#pragma endregion Join

#pragma region Split
    template <typename Str, typename SplitCh, typename Func>
    void Split(const Str &str, const SplitCh &ch, Func &&func)
    {
        const auto len = str.length();
        for (size_t i = 0; i < len && i != str.npos;)
        {
            const auto p = str.find(ch, i);
            auto end = p;
            if (end == str.npos)
                end = len;
            func(std::basic_string_view(str.data() + i, end - i));
            if (p == str.npos)
                break;
            i = p + 1;
        }
    }
#pragma endregion Split

    namespace _Detail
    {
        template <typename StrVec, typename Arg>
        void ArgsToListImpl(StrVec &vec, const Arg &arg)
        {
            vec.push_back(ToStringAs<typename StrVec::value_type>(arg));
        }

        template <typename StrVec, typename... Args>
        void ArgsToList(StrVec &out, Args &&...args)
        {
            (ArgsToListImpl(out, std::forward<Args>(args)), ...);
        }
    } // namespace _Detail

#pragma region Format

    template <typename Str, typename FmtStr, typename... Args>
    void FormatTo(Str &str, const FmtStr &fmtStr, Args &&...args)
    {
        std::vector<Str> argsStr{};
        // const auto fmt = std::filesystem::path(fmtStr).u32string();
        const auto fmt = ToStringAs<Str>(fmtStr);
        _Detail::ArgsToList(argsStr, std::forward<Args>(args)...);
        const auto token = ToStringAs<Str>("{}");
        typename Str::size_type start = 0;
        auto pos = fmt.find(token, start);
        uint64_t i = 0;
        while (pos != decltype(fmt)::npos)
        {
            str.append(ToStringAs<Str>(fmt.substr(start, pos - start)));
            str.append(argsStr.at(i++));
            start = pos + 2;
            pos = fmt.find(token, start);
        }
        str.append(ToStringAs<Str>(fmt.substr(start)));
    }

    template <typename Ret, typename FmtStr, typename... Args>
    [[nodiscard]] Ret FormatAs(const FmtStr &fmtStr, Args &&...args)
    {
        Ret buf{};
        FormatTo(buf, fmtStr, std::forward<Args>(args)...);
        return buf;
    }

#define __FormatImpl(type, func)                                    \
    template <typename FmtStr, typename... Args>                    \
    [[nodiscard]] type func(const FmtStr &fmtStr, Args &&...args)   \
    {                                                               \
        return FormatAs<type>(fmtStr, std::forward<Args>(args)...); \
    }

    __Suit2(__FormatImpl, Format);
#pragma endregion Format

#pragma region Utf8

    template <typename Str>
    [[nodiscard]] auto ToUtf8(const Str &str)
    {
        return std::filesystem::path(str).u8string();
    }

    template <typename Str>
    [[nodiscard]] Str FromUtf8As(const std::string &utf8)
    {
        return _Detail::GetStrFunc<Str>{}(
#ifdef __U8stringUseChar8
            std::filesystem::path
#else
            std::filesystem::u8path
#endif
            (utf8));
    }

#define __FromUtf8Impl(type, func)                          \
    [[nodiscard]] inline type func(const std::string &utf8) \
    {                                                       \
        return FromUtf8As<type>(utf8);                      \
    }

    __Suit2(__FromUtf8Impl, FromUtf8);

#ifdef __U8stringUseChar8
    [[nodiscard]] inline std::u8string_view FromDirtyUtf8String(const std::string_view &str)
    {
        return {reinterpret_cast<const char8_t *>(str.data()), str.length()};
    }

    [[nodiscard]] inline std::string ToDirtyUtf8String(const std::u8string &str)
    {
        return {reinterpret_cast<const char *>(str.c_str()), str.length()};
    }

    [[nodiscard]] inline std::string ToDirtyUtf8String(const std::u8string_view &str)
    {
        return {reinterpret_cast<const char *>(str.data()), str.length()};
    }

    [[nodiscard]] inline std::string_view ToDirtyUtf8StringView(const std::u8string &str)
    {
        return {reinterpret_cast<const char *>(str.c_str()), str.length()};
    }

    [[nodiscard]] inline std::string_view ToDirtyUtf8StringView(const std::u8string_view &str)
    {
        return {reinterpret_cast<const char *>(str.data()), str.length()};
    }
#endif
#pragma endregion Utf8

#pragma region Replace
    template <typename Str, typename OldStr, typename NewStr>
    void ReplaceTo(Str &str, const OldStr &oldStr, const NewStr &newStr)
    {
        size_t i = Str::npos;
        do
        {
            i = str.find(oldStr, i);
            if (i == Str::npos)
                break;

            str.replace(i, oldStr.length(), newStr);
            i += oldStr.length();
        } while (i < str.length());
    }

    template <typename Ret, typename Str, typename OldStr, typename NewStr>
    [[nodiscard]] Ret ReplaceAs(const Str &str, const OldStr &oldStr, const NewStr &newStr)
    {
        Ret buf = str;
        ReplaceTo(buf, oldStr, newStr);
        return buf;
    }

#define __ReplaceImpl(type, func)                                                       \
    template <typename Str, typename OldStr, typename NewStr>                           \
    [[nodiscard]] type func(const Str &str, const OldStr &oldStr, const NewStr &newStr) \
    {                                                                                   \
        return ReplaceAs<type>(str, oldStr, newStr);                                    \
    }

    __Suit2(__ReplaceImpl, Replace);
#pragma endregion Replace
} // namespace String

#undef NewStringImpl
#undef NewString
