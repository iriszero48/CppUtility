#pragma once

#include "IEncoding.hpp"

#include <array>

namespace CuCrypto
{
	namespace Detail::Base85
	{
		static constexpr std::array<char, 86> Table{
			R"(0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz!#$%&()*+-;<=>?@^_`{|}~)"};

		template <uint64_t B, uint64_t E>
		struct Pow
		{
			static constexpr uint64_t Value = B * Pow<B, E - 1>::Value;
		};
		template <uint64_t B>
		struct Pow<B, static_cast<uint64_t>(0)>
		{
			static constexpr uint64_t Value = 1;
		};

		template <uint64_t B, uint64_t E>
		static inline constexpr auto PowV = Pow<B, E>::Value;

		constexpr uint32_t Char4AsUInt(const std::array<uint8_t, 4> buf)
		{
			return (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
		}

		template <std::array<char, 86> Tab>
		constexpr uint32_t Find(const char ch)
		{
			for (size_t i = 0; i < Tab.size() - 1; ++i)
			{
				if (Tab[i] == ch)
					return i;
			}

			throw CuCrypto_MakeExcept("unmap char in Table");
		}

		template <std::array<char, 86> Tab>
		constexpr uint32_t Char5ToUInt(const std::array<char, 5> buf)
		{

			return Find<Tab>(buf[0]) * PowV<85, 4> + Find<Tab>(buf[1]) * PowV<85, 3> + Find<Tab>(buf[2]) * PowV<85, 2> + Find<Tab>(buf[3]) * PowV<85, 1> + Find<Tab>(buf[4]) * PowV<85, 0>;
		}

		template <std::array<char, 86> Tab, bool ReplaceZeros, typename SIt, typename DIt>
		void Encode(SIt begin, SIt end, DIt dest)
		{
			std::array<uint8_t, 4> buf{};
			size_t bufIdx = 0;
			for (auto it = begin; it != end; ++it)
			{
				buf[bufIdx++] = *it;
				if (bufIdx == 4)
				{
					bufIdx = 0;

					if (ReplaceZeros && buf[0] == 0 && buf[1] == 0 && buf[2] == 0 && buf[3] == 0)
					{
						dest++ = 'z';
					}
					else
					{
						const auto intV = Char4AsUInt(buf);

						dest++ = Tab[(intV / PowV<85, 4>) % 85];
						dest++ = Tab[(intV / PowV<85, 3>) % 85];
						dest++ = Tab[(intV / PowV<85, 2>) % 85];
						dest++ = Tab[(intV / PowV<85, 1>) % 85];
						dest++ = Tab[(intV / PowV<85, 0>) % 85];
					}
				}
			}

			if (bufIdx)
			{
				for (size_t i = bufIdx; i < buf.size(); ++i)
				{
					buf[i] = 0;
				}

				const auto intV = Char4AsUInt(buf);
				dest++ = Tab[(intV / PowV<85, 4>) % 85];
				if (bufIdx >= 1)
					dest++ = Tab[(intV / PowV<85, 3>) % 85];
				if (bufIdx >= 2)
					dest++ = Tab[(intV / PowV<85, 2>) % 85];
				if (bufIdx >= 3)
					dest++ = Tab[(intV / PowV<85, 1>) % 85];
			}
		}

		template <std::array<char, 86> Tab, bool ReplaceZeros, typename SIt, typename DIt>
		void Decode(SIt begin, SIt end, DIt dest)
		{
			std::array<char, 5> buf{};
			size_t bufIdx = 0;
			for (auto it = begin; it != end; ++it)
			{
				buf[bufIdx++] = *it;
				if (ReplaceZeros && bufIdx == 1 && buf[0] == 'z')
				{
					bufIdx = 0;

					dest++ = static_cast<uint8_t>(0);
					dest++ = static_cast<uint8_t>(0);
					dest++ = static_cast<uint8_t>(0);
					dest++ = static_cast<uint8_t>(0);
				}
				else if (bufIdx == 5)
				{
					bufIdx = 0;
					const auto intV = Char5ToUInt<Tab>(buf);

					dest++ = static_cast<uint8_t>((intV >> 24) & 0xff);
					dest++ = static_cast<uint8_t>((intV >> 16) & 0xff);
					dest++ = static_cast<uint8_t>((intV >> 8) & 0xff);
					dest++ = static_cast<uint8_t>((intV >> 0) & 0xff);
				}
			}

			if (bufIdx)
			{
				for (size_t i = bufIdx; i < buf.size(); ++i)
				{
					buf[i] = Tab[84];
				}
				const auto intV = Char5ToUInt<Tab>(buf);
				if (bufIdx >= 2)
					dest++ = static_cast<uint8_t>((intV >> 24) & 0xff);
				if (bufIdx >= 3)
					dest++ = static_cast<uint8_t>((intV >> 16) & 0xff);
				if (bufIdx >= 4)
					dest++ = static_cast<uint8_t>((intV >> 8) & 0xff);
			}
		}

		template <std::array<char, 86> Tab, bool ReplaceZeros>
		class Base85Base : public IEncoding<Base85Base<Tab, ReplaceZeros>>
		{
		public:
			template <typename SIt, typename DIt>
			void EncodeCore(SIt begin, SIt end, DIt dest)
			{
				Base85::Encode<Tab, ReplaceZeros>(begin, end, dest);
			}

			template <typename SIt, typename DIt>
			void DecodeCore(SIt begin, SIt end, DIt dest)
			{
				Base85::Decode<Tab, ReplaceZeros>(begin, end, dest);
			}
		};
	}

	using Base85 = Detail::Base85::Base85Base<Detail::Base85::Table, false>;
}