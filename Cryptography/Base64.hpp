#pragma once

#include "Base85.hpp"

namespace CuCrypto
{
	namespace Detail::Base64
	{
		static constexpr std::array<char, 65> Table{"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"};

		constexpr std::tuple<char, char, char, char> _3To4(const std::array<uint8_t, 3> &buf)
		{
			const auto [a, b, c] = buf;
			return std::make_tuple(
				Table[a >> 2],
				Table[(a & 0x3) << 4 | b >> 4],
				Table[(b & 0xf) << 2 | c >> 6],
				Table[c & 0x3f]);
		}

		constexpr uint8_t Find(const char ch)
		{
			for (uint8_t i = 0; i < Table.size() - 1; ++i)
			{
				if (Table[i] == ch)
					return i;
			}

			throw CuCrypto_MakeExcept("unmap char in Table");
		}

		constexpr std::tuple<uint8_t, uint8_t, uint8_t> _4To3(const std::array<char, 4> &buf)
		{
			const auto a = Find(buf[0]);
			const auto b = Find(buf[1]);
			const auto c = Find(buf[2]);
			const auto d = Find(buf[3]);

			return std::make_tuple(
				static_cast<uint8_t>(a << 2 | b >> 4),
				static_cast<uint8_t>(b << 4 | c >> 2),
				static_cast<uint8_t>(c << 6 | d));
		}
	}

	class Base64 : public IEncoding<Base64>
	{
	private:
		bool padEnd = false;

	public:
		Base64(bool pad = false) : padEnd(pad) {}

		template <typename SIt, typename DIt>
		void EncodeCore(SIt begin, SIt end, DIt dest)
		{
			std::array<uint8_t, 3> buf{};
			size_t bufIdx = 0;
			for (auto it = begin; it != end; ++it)
			{
				buf[bufIdx++] = *it;
				if (bufIdx == 3)
				{
					bufIdx = 0;

					const auto [a, b, c, d] = Detail::Base64::_3To4(buf);

					dest++ = a;
					dest++ = b;
					dest++ = c;
					dest++ = d;
				}
			}

			if (bufIdx)
			{
				for (size_t i = bufIdx; i < buf.size(); ++i)
				{
					buf[i] = 0;
				}

				const auto [a, b, c, d] = Detail::Base64::_3To4(buf);

				dest++ = a;
				dest++ = b;
				if (bufIdx >= 2)
				{
					dest++ = c;
					if (padEnd)
						dest++ = '=';
				}
				else if (padEnd)
				{
					dest++ = '=';
					dest++ = '=';
				}
			}
		}

		template <typename SIt, typename DIt>
		void DecodeCore(SIt begin, SIt end, DIt dest)
		{
			std::array<char, 4> buf{};
			size_t bufIdx = 0;
			for (auto it = begin; it != end; ++it)
			{
				if (const auto cc = *it; cc != '=')
				{
					buf[bufIdx++] = cc;
					if (bufIdx == 4)
					{
						bufIdx = 0;

						const auto [a, b, c] = Detail::Base64::_4To3(buf);

						dest++ = a;
						dest++ = b;
						dest++ = c;
					}
				}
			}

			if (bufIdx)
			{
				for (size_t i = bufIdx; i < buf.size(); ++i)
				{
					buf[i] = 0;
				}

				const auto [a, b, c] = Detail::Base64::_4To3(buf);

				if (bufIdx >= 2)
					dest++ = a;
				if (bufIdx >= 3)
					dest++ = b;
			}
		}
	};
}