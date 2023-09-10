#pragma once

#include "IHash.hpp"
#include "IHashAlgorithm.hpp"
#include "Utility.hpp"
#include "Bit/Bit.hpp"

namespace CuCrypto
{
	namespace Detail::Md5
	{
		enum class Round
		{
			F,
			G,
			H,
			I
		};

		constexpr std::uint32_t F(const std::uint32_t x, const std::uint32_t y, const std::uint32_t z)
		{
			return z ^ x & (y ^ z);
		}

		constexpr std::uint32_t G(const std::uint32_t x, const std::uint32_t y, const std::uint32_t z)
		{
			return y ^ z & (x ^ y);
		}

		constexpr std::uint32_t H(const std::uint32_t x, const std::uint32_t y, const std::uint32_t z)
		{
			return x ^ y ^ z;
		}

		constexpr std::uint32_t I(const std::uint32_t x, const std::uint32_t y, const std::uint32_t z)
		{
			return y ^ (x | ~z);
		}

		template <Round Func>
		constexpr void Step(std::uint32_t &a, const std::uint32_t b, const std::uint32_t c, const std::uint32_t d, const std::uint32_t x, const std::uint32_t t, const std::uint32_t s)
		{
			if constexpr (Func == Round::F)
				a += F(b, c, d) + x + t;
			else if constexpr (Func == Round::G)
				a += G(b, c, d) + x + t;
			else if constexpr (Func == Round::H)
				a += H(b, c, d) + x + t;
			else if constexpr (Func == Round::I)
				a += I(b, c, d) + x + t;
			a = a << s | (a >> (32u - s));
			a += b;
		}

		inline std::uint32_t Get(const std::uint8_t *buf, const std::uint64_t index)
		{
			if constexpr (CuBit::Endian::Native == CuBit::Endian::Little)
				return *reinterpret_cast<const std::uint32_t *>(&buf[index * 4]);
			else
				return CuBit::ByteSwap(*reinterpret_cast<const std::uint32_t *>(&buf[index * 4]));
		}
	}

	class Md5 : public IHashAlgorithm<Md5, std::array<uint8_t, 16>>
	{
	public:
		using HashValueType = HashValueType;

		Md5() : data({{0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476}}) {}

		void AppendCore(const std::uint8_t *buf, std::uint64_t len)
		{
			if (bufferLen != 0)
			{
				const std::uint64_t emp = 64u - bufferLen;
				if (len < emp)
				{
					std::copy_n(buf, len, buffer + bufferLen);
					bufferLen += static_cast<std::uint8_t>(len);
					return;
				}

				std::copy_n(buf, emp, buffer + bufferLen);
				buf += emp;
				len -= emp;
				Append64(data, buffer, 1);
				length += 64;
				bufferLen = 0;
			}

			const auto n = len >> 6u; // = len / 64
			const auto nByte = n * 64u;
			Append64(data, buf, n);
			length += nByte;
			bufferLen = len & 0x3fu; // = len % 64
			if (bufferLen != 0)
				std::copy_n(buf + nByte, bufferLen, buffer);
		}

		[[nodiscard]] HashType Digest() const
		{
			using namespace CuBit;

			auto len = length;
			auto bufLen = bufferLen;
			std::uint8_t buf[64];
			std::copy_n(buffer, bufLen, buf);
			auto dat = data;

			len += bufLen;
			len <<= 3u; // *= 8
			buf[bufLen++] = 0x80u;

			auto emp = 64u - bufLen;
			if (emp < 8u)
			{
				memset(buf + bufLen, 0u, emp);
				Append64(dat, buf, 1);
				bufLen = 0;
				emp = 64u;
			}
			memset(buf + bufLen, 0, emp - 8u);

			if constexpr (Endian::Native == Endian::Big)
				len = ByteSwap(len);
			memcpy(buf + 56, &len, 8);
			Append64(dat, buf, 1);

			if constexpr (Endian::Native == Endian::Big)
			{
				dat.DWord.A = ByteSwap(dat.DWord.A);
				dat.DWord.B = ByteSwap(dat.DWord.B);
				dat.DWord.C = ByteSwap(dat.DWord.C);
				dat.DWord.D = ByteSwap(dat.DWord.D);
			}
			return Detail::Hash::CreateArray(dat.Word);
		}

	private:
		union DigestData
		{
			struct Integer
			{
				std::uint32_t A, B, C, D;
			} DWord;
			std::uint8_t Word[16];
		};

		DigestData data;
		std::uint8_t buffer[64]{0};
		std::uint8_t bufferLen = 0;
		std::uint64_t length = 0;

		void Append64(DigestData &dat, const std::uint8_t *buf, std::uint64_t n) const
		{
			using namespace Detail::Md5;
			auto [a, b, c, d] = dat.DWord;

			while (n--)
			{
				const auto savedA = a;
				const auto savedB = b;
				const auto savedC = c;
				const auto savedD = d;

				Step<Round::F>(a, b, c, d, Get(buf, 0), 0xd76aa478u, 7);
				Step<Round::F>(d, a, b, c, Get(buf, 1), 0xe8c7b756u, 12);
				Step<Round::F>(c, d, a, b, Get(buf, 2), 0x242070dbu, 17);
				Step<Round::F>(b, c, d, a, Get(buf, 3), 0xc1bdceeeu, 22);
				Step<Round::F>(a, b, c, d, Get(buf, 4), 0xf57c0fafu, 7);
				Step<Round::F>(d, a, b, c, Get(buf, 5), 0x4787c62au, 12);
				Step<Round::F>(c, d, a, b, Get(buf, 6), 0xa8304613u, 17);
				Step<Round::F>(b, c, d, a, Get(buf, 7), 0xfd469501u, 22);
				Step<Round::F>(a, b, c, d, Get(buf, 8), 0x698098d8u, 7);
				Step<Round::F>(d, a, b, c, Get(buf, 9), 0x8b44f7afu, 12);
				Step<Round::F>(c, d, a, b, Get(buf, 10), 0xffff5bb1u, 17);
				Step<Round::F>(b, c, d, a, Get(buf, 11), 0x895cd7beu, 22);
				Step<Round::F>(a, b, c, d, Get(buf, 12), 0x6b901122u, 7);
				Step<Round::F>(d, a, b, c, Get(buf, 13), 0xfd987193u, 12);
				Step<Round::F>(c, d, a, b, Get(buf, 14), 0xa679438eu, 17);
				Step<Round::F>(b, c, d, a, Get(buf, 15), 0x49b40821u, 22);

				Step<Round::G>(a, b, c, d, Get(buf, 1), 0xf61e2562u, 5);
				Step<Round::G>(d, a, b, c, Get(buf, 6), 0xc040b340u, 9);
				Step<Round::G>(c, d, a, b, Get(buf, 11), 0x265e5a51u, 14);
				Step<Round::G>(b, c, d, a, Get(buf, 0), 0xe9b6c7aau, 20);
				Step<Round::G>(a, b, c, d, Get(buf, 5), 0xd62f105du, 5);
				Step<Round::G>(d, a, b, c, Get(buf, 10), 0x02441453u, 9);
				Step<Round::G>(c, d, a, b, Get(buf, 15), 0xd8a1e681u, 14);
				Step<Round::G>(b, c, d, a, Get(buf, 4), 0xe7d3fbc8u, 20);
				Step<Round::G>(a, b, c, d, Get(buf, 9), 0x21e1cde6u, 5);
				Step<Round::G>(d, a, b, c, Get(buf, 14), 0xc33707d6u, 9);
				Step<Round::G>(c, d, a, b, Get(buf, 3), 0xf4d50d87u, 14);
				Step<Round::G>(b, c, d, a, Get(buf, 8), 0x455a14edu, 20);
				Step<Round::G>(a, b, c, d, Get(buf, 13), 0xa9e3e905u, 5);
				Step<Round::G>(d, a, b, c, Get(buf, 2), 0xfcefa3f8u, 9);
				Step<Round::G>(c, d, a, b, Get(buf, 7), 0x676f02d9u, 14);
				Step<Round::G>(b, c, d, a, Get(buf, 12), 0x8d2a4c8au, 20);

				Step<Round::H>(a, b, c, d, Get(buf, 5), 0xfffa3942u, 4);
				Step<Round::H>(d, a, b, c, Get(buf, 8), 0x8771f681u, 11);
				Step<Round::H>(c, d, a, b, Get(buf, 11), 0x6d9d6122u, 16);
				Step<Round::H>(b, c, d, a, Get(buf, 14), 0xfde5380cu, 23);
				Step<Round::H>(a, b, c, d, Get(buf, 1), 0xa4beea44u, 4);
				Step<Round::H>(d, a, b, c, Get(buf, 4), 0x4bdecfa9u, 11);
				Step<Round::H>(c, d, a, b, Get(buf, 7), 0xf6bb4b60u, 16);
				Step<Round::H>(b, c, d, a, Get(buf, 10), 0xbebfbc70u, 23);
				Step<Round::H>(a, b, c, d, Get(buf, 13), 0x289b7ec6u, 4);
				Step<Round::H>(d, a, b, c, Get(buf, 0), 0xeaa127fau, 11);
				Step<Round::H>(c, d, a, b, Get(buf, 3), 0xd4ef3085u, 16);
				Step<Round::H>(b, c, d, a, Get(buf, 6), 0x04881d05u, 23);
				Step<Round::H>(a, b, c, d, Get(buf, 9), 0xd9d4d039u, 4);
				Step<Round::H>(d, a, b, c, Get(buf, 12), 0xe6db99e5u, 11);
				Step<Round::H>(c, d, a, b, Get(buf, 15), 0x1fa27cf8u, 16);
				Step<Round::H>(b, c, d, a, Get(buf, 2), 0xc4ac5665u, 23);

				Step<Round::I>(a, b, c, d, Get(buf, 0), 0xf4292244u, 6);
				Step<Round::I>(d, a, b, c, Get(buf, 7), 0x432aff97u, 10);
				Step<Round::I>(c, d, a, b, Get(buf, 14), 0xab9423a7u, 15);
				Step<Round::I>(b, c, d, a, Get(buf, 5), 0xfc93a039u, 21);
				Step<Round::I>(a, b, c, d, Get(buf, 12), 0x655b59c3u, 6);
				Step<Round::I>(d, a, b, c, Get(buf, 3), 0x8f0ccc92u, 10);
				Step<Round::I>(c, d, a, b, Get(buf, 10), 0xffeff47du, 15);
				Step<Round::I>(b, c, d, a, Get(buf, 1), 0x85845dd1u, 21);
				Step<Round::I>(a, b, c, d, Get(buf, 8), 0x6fa87e4fu, 6);
				Step<Round::I>(d, a, b, c, Get(buf, 15), 0xfe2ce6e0u, 10);
				Step<Round::I>(c, d, a, b, Get(buf, 6), 0xa3014314u, 15);
				Step<Round::I>(b, c, d, a, Get(buf, 13), 0x4e0811a1u, 21);
				Step<Round::I>(a, b, c, d, Get(buf, 4), 0xf7537e82u, 6);
				Step<Round::I>(d, a, b, c, Get(buf, 11), 0xbd3af235u, 10);
				Step<Round::I>(c, d, a, b, Get(buf, 2), 0x2ad7d2bbu, 15);
				Step<Round::I>(b, c, d, a, Get(buf, 9), 0xeb86d391u, 21);

				a += savedA;
				b += savedB;
				c += savedC;
				d += savedD;

				buf += 64;
			}
			dat.DWord.A = a;
			dat.DWord.B = b;
			dat.DWord.C = c;
			dat.DWord.D = d;
		}
	};

	CuCrypto_MakeU8ArrayHashImpl(Md5::HashValueType);
}
