#pragma once

#include "Exception.hpp"

#include <climits>
#include <stdexcept>

namespace CuCrypto::Detail
{
	namespace Hash
	{
		template <size_t S>
		void Uint8ArrayToStringPaddingZero(const std::array<uint8_t, S>& data, std::string& hex)
		{
			for (const auto value : data)
			{
				char res[4]{ '0', 0, 0, 0 };
				if (const auto [p, e] = std::to_chars(res + 1, res + 3, value, 16);
					e != std::errc{})
					throw Exception("convert error");
				hex.append(res[2] == 0 ? std::string_view(res, 2) : std::string_view(res + 1, 2));
			}
		}

		template <typename T>
		void ToStringPaddingZero(const T value, std::string& hex)
		{
			constexpr auto size = sizeof(T);
			constexpr auto bufSize = size * 2 + 1;

			std::string res(bufSize, 0);
			if (const auto [p, e] = std::to_chars(res.data(), res.data() + bufSize - 1, value, 16);
				e != std::errc{})
				throw CuCrypto_MakeExcept("convert error");

			const std::string_view revSv(res.data());
			if (const auto padSz = bufSize - 1 - revSv.length(); padSz)
                hex.append(padSz, '0');
			hex.append(revSv);
		}

		template <typename T>
		inline bool ArrayCmp(const T& l, const T& r)
		{
			return std::equal(std::begin(l), std::end(l), std::begin(r));
		}

		template <typename T>
		inline bool HashStrCmp(const T& hash, const std::string& str)
		{
			return hash.ToString() == CuStr::ToLower(str);
		}

		template <typename T, size_t S>
		std::array<T, S> CreateArray(const T(&data)[S])
		{
			std::array<T, S> buf{};
			std::copy_n(data, S, buf.data());
			return buf;
		}
	}

	namespace Bit
	{
		template <typename T>
		constexpr T RotateLeft(const T x, const int n)
		{
			return (x << n) | (x >> (sizeof(T) * 8 - n));
		}

		template <typename T>
		constexpr T RotateRight(const T x, const int n)
		{
			return (x >> n) | (x << (sizeof(T) * 8 - n));
		}
	}
}
