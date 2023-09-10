#pragma once

#include <charconv>
#include <optional>
#include <string>

#include "../Utility/Utility.hpp"

#ifndef CuUtil_Compiler_MSVC
#include <sstream>
#endif

namespace CuConv
{
	namespace Detail
	{
#ifndef CuUtil_Compiler_MSVC
		template <typename T>
		[[nodiscard]] std::optional<std::string> ToStringStmImpl(const T &value) noexcept
		{
			try
			{
				std::ostringstream ss;
				ss << value;
				return ss.str();
			}
			catch (...)
			{
				return std::nullopt;
			}
		}

		template <typename T>
		[[nodiscard]] std::optional<T> FromStringStmImpl(const std::string &value) noexcept
		{
			try
			{
				T val;
				std::stringstream ss(value);
				ss >> val;
				return val;
			}
			catch (...)
			{
				return std::nullopt;
			}
		}
#endif

		template <typename T, typename Args>
		[[nodiscard]] std::optional<T> FromStringImpl(const std::string_view &value, const Args args) noexcept
		{
			T res{};
			const auto begin = value.data();
			const auto end = begin + value.length();
			if (auto [p, e] = std::from_chars(begin, end, res, args); e != std::errc{})
				return {};
			return res;
		}

		template <typename T, typename... Args>
		[[nodiscard]] std::optional<std::string> ToStringImpl(const T &value, Args &&...args) noexcept
		{
			char res[sizeof(T) * 8 + 1] = {0};
			if (auto [p, e] = std::to_chars(res, res + 65, value, std::forward<Args>(args)...); e != std::errc{})
				return {};
			return res;
		}
	}

	template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
	[[nodiscard]] decltype(auto) ToString(const T value, const int base = 10) noexcept
	{
		return Detail::ToStringImpl<T>(value, base);
	}

#ifdef CuUtil_Compiler_MSVC
	template <typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
	[[nodiscard]] decltype(auto) ToString(const T value) noexcept
	{
		return Detail::ToStringImpl<T>(value);
	}

	template <typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
	[[nodiscard]] decltype(auto) ToString(const T value, const std::chars_format &fmt) noexcept
	{
		return Detail::ToStringImpl<T>(value, fmt);
	}

	template <typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
	[[nodiscard]] decltype(auto) ToString(const T value, const std::chars_format &fmt, const int precision) noexcept
	{
		return Detail::ToStringImpl<T>(value, fmt, precision);
	}
#else
	template <typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
	[[nodiscard]] decltype(auto) ToString(const T value) noexcept
	{
		return Detail::ToStringStmImpl<T>(value);
	}
#endif

	template <typename T, typename Str, std::enable_if_t<std::is_integral_v<T>, int> = 0>
	[[nodiscard]] decltype(auto) FromString(const Str value, const int base = 10) noexcept
	{
		return Detail::FromStringImpl<T>(value, base);
	}

#ifdef CuUtil_Compiler_MSVC
	template <typename T, typename Str, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
	[[nodiscard]] decltype(auto) FromString(const Str value,
											const std::chars_format &fmt = std::chars_format::general) noexcept
	{
		return Detail::FromStringImpl<T>(value, fmt);
	}
#else
	template <typename T, typename Str, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
	[[nodiscard]] decltype(auto) FromString(const Str &value) noexcept
	{
		return Detail::FromStringStmImpl<T>(std::string(value));
	}
#endif
}