#pragma once

#include "IHashAlgorithm.hpp"

namespace CuCrypto
{
	namespace Detail::Sha1
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
			return z ^ (x & (y ^ z));
		}

		constexpr std::uint32_t G(const std::uint32_t x, const std::uint32_t y, const std::uint32_t z)
		{
			return x ^ y ^ z;
		}

		constexpr std::uint32_t H(const std::uint32_t x, const std::uint32_t y, const std::uint32_t z)
		{
			return (x & y) | (z & (x | y));
		}

		constexpr std::uint32_t I(const std::uint32_t x, const std::uint32_t y, const std::uint32_t z)
		{
			return x ^ y ^ z;
		}

		constexpr std::uint32_t WAt(const std::uint32_t *w, const std::uint8_t i)
		{
			return w[i & 15u];
		}

		constexpr std::uint32_t W(std::uint32_t *w, const std::uint8_t i)
		{
			using namespace Detail::Bit;

			w[i & 15] = RotateLeft(WAt(w, i - 3) ^ WAt(w, i - 8) ^ WAt(w, i - 14) ^ WAt(w, i - 16), 1);
			return w[i & 15];
		}

		template <Round Func>
		constexpr void Step(std::uint32_t &a, std::uint32_t &b, const std::uint32_t c, const std::uint32_t d, std::uint32_t &e, const std::uint32_t w)
		{
			using namespace Detail::Bit;

			if constexpr (Func == Round::F)
				e += F(b, c, d) + w + 0x5A827999u;
			else if constexpr (Func == Round::G)
				e += G(b, c, d) + w + 0x6ED9EBA1u;
			else if constexpr (Func == Round::H)
				e += H(b, c, d) + w + 0x8F1BBCDCu;
			else if constexpr (Func == Round::I)
				e += I(b, c, d) + w + 0xCA62C1D6u;
			e += RotateLeft(a, 5);
			b = RotateLeft(b, 30);
		}
	}

	class Sha1 : public IHashAlgorithm<Sha1, std::array<uint8_t, 20>>
	{
	public:
		using HashValueType = HashValueType;

		Sha1() : data({{0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476, 0xc3d2e1f0}}) {}

		void AppendCore(const std::uint8_t *buf, std::uint64_t len)
		{
			if (bufferLen != 0)
			{
				const std::uint64_t emp = 64u - bufferLen;
				if (len < emp)
				{
					std::copy_n(buf, len, buffer + bufferLen);
					bufferLen += len;
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

		HashType Digest() const
		{
			using namespace CuBit;

			DigestData dat = data;
			auto len = length;
			auto bufLen = bufferLen;
			std::uint8_t buf[64];
			std::copy_n(buffer, bufLen, buf);

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

			if constexpr (Endian::Native == Endian::Little)
				len = ByteSwap(len);
			memcpy(buf + 56, &len, 8);
			Append64(dat, buf, 1);

			if constexpr (Endian::Native == Endian::Little)
			{
				dat.DWord.A = ByteSwap(dat.DWord.A);
				dat.DWord.B = ByteSwap(dat.DWord.B);
				dat.DWord.C = ByteSwap(dat.DWord.C);
				dat.DWord.D = ByteSwap(dat.DWord.D);
				dat.DWord.E = ByteSwap(dat.DWord.E);
			}
			return Detail::Hash::CreateArray(dat.Word);
		}

	private:
		union DigestData
		{
			struct Integer
			{
				std::uint32_t A, B, C, D, E;
			} DWord;
			std::uint8_t Word[20];
		};

		DigestData data;
		std::uint8_t buffer[64]{0};
		std::uint8_t bufferLen = 0;
		std::uint64_t length = 0;

		void Append64(DigestData &dat, const std::uint8_t *buf, std::uint64_t n) const
		{
			using namespace Detail::Sha1;
			using namespace CuBit;

			auto [a, b, c, d, e] = dat.DWord;

			while (n--)
			{
				const auto savedA = a;
				const auto savedB = b;
				const auto savedC = c;
				const auto savedD = d;
				const auto savedE = e;

				std::uint32_t w[16];
				for (int i = 0; i < 16; ++i)
				{
					if constexpr (Endian::Native == Endian::Little)
						w[i] = ByteSwap(*(std::uint32_t *)&buf[i * 4]);
					if constexpr (Endian::Native == Endian::Big)
						w[i] = (*(std::uint32_t *)&buf[i * 4]);
				}

				Step<Round::F>(a, b, c, d, e, w[0]);
				Step<Round::F>(e, a, b, c, d, w[1]);
				Step<Round::F>(d, e, a, b, c, w[2]);
				Step<Round::F>(c, d, e, a, b, w[3]);
				Step<Round::F>(b, c, d, e, a, w[4]);
				Step<Round::F>(a, b, c, d, e, w[5]);
				Step<Round::F>(e, a, b, c, d, w[6]);
				Step<Round::F>(d, e, a, b, c, w[7]);
				Step<Round::F>(c, d, e, a, b, w[8]);
				Step<Round::F>(b, c, d, e, a, w[9]);
				Step<Round::F>(a, b, c, d, e, w[10]);
				Step<Round::F>(e, a, b, c, d, w[11]);
				Step<Round::F>(d, e, a, b, c, w[12]);
				Step<Round::F>(c, d, e, a, b, w[13]);
				Step<Round::F>(b, c, d, e, a, w[14]);
				Step<Round::F>(a, b, c, d, e, w[15]);
				Step<Round::F>(e, a, b, c, d, W(w, 16));
				Step<Round::F>(d, e, a, b, c, W(w, 17));
				Step<Round::F>(c, d, e, a, b, W(w, 18));
				Step<Round::F>(b, c, d, e, a, W(w, 19));

				Step<Round::G>(a, b, c, d, e, W(w, 20));
				Step<Round::G>(e, a, b, c, d, W(w, 21));
				Step<Round::G>(d, e, a, b, c, W(w, 22));
				Step<Round::G>(c, d, e, a, b, W(w, 23));
				Step<Round::G>(b, c, d, e, a, W(w, 24));
				Step<Round::G>(a, b, c, d, e, W(w, 25));
				Step<Round::G>(e, a, b, c, d, W(w, 26));
				Step<Round::G>(d, e, a, b, c, W(w, 27));
				Step<Round::G>(c, d, e, a, b, W(w, 28));
				Step<Round::G>(b, c, d, e, a, W(w, 29));
				Step<Round::G>(a, b, c, d, e, W(w, 30));
				Step<Round::G>(e, a, b, c, d, W(w, 31));
				Step<Round::G>(d, e, a, b, c, W(w, 32));
				Step<Round::G>(c, d, e, a, b, W(w, 33));
				Step<Round::G>(b, c, d, e, a, W(w, 34));
				Step<Round::G>(a, b, c, d, e, W(w, 35));
				Step<Round::G>(e, a, b, c, d, W(w, 36));
				Step<Round::G>(d, e, a, b, c, W(w, 37));
				Step<Round::G>(c, d, e, a, b, W(w, 38));
				Step<Round::G>(b, c, d, e, a, W(w, 39));

				Step<Round::H>(a, b, c, d, e, W(w, 40));
				Step<Round::H>(e, a, b, c, d, W(w, 41));
				Step<Round::H>(d, e, a, b, c, W(w, 42));
				Step<Round::H>(c, d, e, a, b, W(w, 43));
				Step<Round::H>(b, c, d, e, a, W(w, 44));
				Step<Round::H>(a, b, c, d, e, W(w, 45));
				Step<Round::H>(e, a, b, c, d, W(w, 46));
				Step<Round::H>(d, e, a, b, c, W(w, 47));
				Step<Round::H>(c, d, e, a, b, W(w, 48));
				Step<Round::H>(b, c, d, e, a, W(w, 49));
				Step<Round::H>(a, b, c, d, e, W(w, 50));
				Step<Round::H>(e, a, b, c, d, W(w, 51));
				Step<Round::H>(d, e, a, b, c, W(w, 52));
				Step<Round::H>(c, d, e, a, b, W(w, 53));
				Step<Round::H>(b, c, d, e, a, W(w, 54));
				Step<Round::H>(a, b, c, d, e, W(w, 55));
				Step<Round::H>(e, a, b, c, d, W(w, 56));
				Step<Round::H>(d, e, a, b, c, W(w, 57));
				Step<Round::H>(c, d, e, a, b, W(w, 58));
				Step<Round::H>(b, c, d, e, a, W(w, 59));

				Step<Round::I>(a, b, c, d, e, W(w, 60));
				Step<Round::I>(e, a, b, c, d, W(w, 61));
				Step<Round::I>(d, e, a, b, c, W(w, 62));
				Step<Round::I>(c, d, e, a, b, W(w, 63));
				Step<Round::I>(b, c, d, e, a, W(w, 64));
				Step<Round::I>(a, b, c, d, e, W(w, 65));
				Step<Round::I>(e, a, b, c, d, W(w, 66));
				Step<Round::I>(d, e, a, b, c, W(w, 67));
				Step<Round::I>(c, d, e, a, b, W(w, 68));
				Step<Round::I>(b, c, d, e, a, W(w, 69));
				Step<Round::I>(a, b, c, d, e, W(w, 70));
				Step<Round::I>(e, a, b, c, d, W(w, 71));
				Step<Round::I>(d, e, a, b, c, W(w, 72));
				Step<Round::I>(c, d, e, a, b, W(w, 73));
				Step<Round::I>(b, c, d, e, a, W(w, 74));
				Step<Round::I>(a, b, c, d, e, W(w, 75));
				Step<Round::I>(e, a, b, c, d, W(w, 76));
				Step<Round::I>(d, e, a, b, c, W(w, 77));
				Step<Round::I>(c, d, e, a, b, W(w, 78));
				Step<Round::I>(b, c, d, e, a, W(w, 79));

				a += savedA;
				b += savedB;
				c += savedC;
				d += savedD;
				e += savedE;

				buf += 64;
			}

			dat.DWord.A = a;
			dat.DWord.B = b;
			dat.DWord.C = c;
			dat.DWord.D = d;
			dat.DWord.E = e;
		}
	};

	CuCrypto_MakeU8ArrayHashImpl(Sha1::HashValueType);
}