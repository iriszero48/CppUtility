#pragma once

#include <cstdint>
#include <type_traits>
#include <climits>
#include <utility>

#include "../Utility/Utility.hpp"

namespace CuBit
{
	enum class Endian
	{
#if defined(__cpp_lib_endian) && !defined(__APPLE__)
		Little = static_cast<std::underlying_type_t<std::endian>>(std::endian::little),
		Big = static_cast<std::underlying_type_t<std::endian>>(std::endian::big),
		Native = static_cast<std::underlying_type_t<std::endian>>(std::endian::native)
#elif CuUtil_Platform_Windows
		Little = 0,
		Big = 1,
		Native = Little
#else
		Little = __ORDER_LITTLE_ENDIAN__,
		Big = __ORDER_BIG_ENDIAN__,
		Native = __BYTE_ORDER__
#endif
	};

	namespace Detail
	{
		template <typename T, std::size_t... N>
		constexpr T ByteSwapImpl(T i, std::index_sequence<N...>)
		{
			return (((i >> N * CHAR_BIT & static_cast<std::uint8_t>(-1)) << (sizeof(T) - 1 - N) * CHAR_BIT) | ...);
		}

		template <std::size_t S>
		struct IntType
		{
		};
		template <>
		struct IntType<1>
		{
			using Value = std::uint8_t;
		};
		template <>
		struct IntType<2>
		{
			using Value = std::uint16_t;
		};
		template <>
		struct IntType<4>
		{
			using Value = std::uint32_t;
		};
		template <>
		struct IntType<8>
		{
			using Value = std::uint64_t;
		};
	}

	template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0, typename U = std::make_unsigned_t<T>>
	constexpr T ByteSwap(T i)
	{
		return Detail::ByteSwapImpl<U>(i, std::make_index_sequence<sizeof(T)>{});
	}

	template <typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
	T ByteSwap(T value) noexcept
	{
		using IntT = typename Detail::IntType<sizeof(T)>::Value;
		const auto uv = *reinterpret_cast<IntT *>(&value);
		const auto ret = EndianSwap(uv);
		return *reinterpret_cast<const T *>(&ret);
	}
}
