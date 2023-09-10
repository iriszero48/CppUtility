#pragma once

#include "IHashAlgorithm.hpp"

namespace CuCrypto
{
	namespace Detail::Sha256
	{
		static std::uint32_t K[] =
			{
				0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
				0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
				0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
				0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
				0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
				0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
				0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
				0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};

		constexpr std::uint32_t Ch(const std::uint32_t x, const std::uint32_t y, const std::uint32_t z)
		{
			return (x & y) ^ (~x & z);
		}

		constexpr std::uint32_t Maj(const std::uint32_t x, const std::uint32_t y, const std::uint32_t z)
		{
			return (x & y) ^ (x & z) ^ (y & z);
		}

		constexpr std::uint32_t S0(const std::uint32_t x)
		{
			return Bit::RotateRight(x, 2) ^ Bit::RotateRight(x, 13) ^ Bit::RotateRight(x, 22);
		}

		constexpr std::uint32_t S1(const std::uint32_t x)
		{
			return Bit::RotateRight(x, 6) ^ Bit::RotateRight(x, 11) ^ Bit::RotateRight(x, 25);
		}

		constexpr std::uint32_t R0(const std::uint32_t x)
		{
			return Bit::RotateRight(x, 7) ^ Bit::RotateRight(x, 18) ^ (x >> 3);
		}

		constexpr std::uint32_t R1(const std::uint32_t x)
		{
			return Bit::RotateRight(x, 17) ^ Bit::RotateRight(x, 19) ^ (x >> 10);
		}
	}

	class Sha256 : public IHashAlgorithm<Sha256, std::array<uint8_t, 32>>
	{
	public:
		using HashValueType = HashValueType;

		Sha256() : data({{0x6a09e667u,
						  0xbb67ae85u,
						  0x3c6ef372u,
						  0xa54ff53au,
						  0x510e527fu,
						  0x9b05688cu,
						  0x1f83d9abu,
						  0x5be0cd19u}}) {}

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

			auto dat = data;
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
				dat.DWord.F = ByteSwap(dat.DWord.F);
				dat.DWord.G = ByteSwap(dat.DWord.G);
				dat.DWord.H = ByteSwap(dat.DWord.H);
			}
			return Detail::Hash::CreateArray(dat.Word);
		}

	private:
		union DigestData
		{
			struct Integer
			{
				std::uint32_t A, B, C, D, E, F, G, H;
			} DWord;
			std::uint8_t Word[32];
		};

		DigestData data;
		std::uint8_t buffer[64]{0};
		std::uint8_t bufferLen = 0;
		std::uint64_t length = 0;

		void Append64(DigestData &dat, const std::uint8_t *buf, std::uint64_t n) const
		{
			using namespace Detail::Sha256;
			using namespace CuBit;

			auto [a, b, c, d, e, f, g, h] = dat.DWord;

			while (n--)
			{
				const auto savedA = a;
				const auto savedB = b;
				const auto savedC = c;
				const auto savedD = d;
				const auto savedE = e;
				const auto savedF = f;
				const auto savedG = g;
				const auto savedH = h;

				std::uint32_t w[64];
				for (int i = 0; i < 16; ++i)
				{
					if constexpr (Endian::Native == Endian::Little)
						w[i] = ByteSwap(*(std::uint32_t *)&buf[i * 4]);
					if constexpr (Endian::Native == Endian::Big)
						w[i] = (*(std::uint32_t *)&buf[i * 4]);
				}

				for (auto i = 16; i < 64; ++i)
				{
					w[i] = R1(w[(i - 2)]) + w[(i - 7)] + R0(w[(i - 15)]) + w[(i - 16)];
				}

				for (auto i = 0; i < 64; ++i)
				{
					const auto temp1 = h + S1(e) + Ch(e, f, g) + K[i] + w[i];
					const auto temp2 = S0(a) + Maj(a, b, c);
					h = g;
					g = f;
					f = e;
					e = d + temp1;
					d = c;
					c = b;
					b = a;
					a = temp1 + temp2;
				}

				a += savedA;
				b += savedB;
				c += savedC;
				d += savedD;
				e += savedE;
				f += savedF;
				g += savedG;
				h += savedH;

				buf += 64;
			}

			dat.DWord.A = a;
			dat.DWord.B = b;
			dat.DWord.C = c;
			dat.DWord.D = d;
			dat.DWord.E = e;
			dat.DWord.F = f;
			dat.DWord.G = g;
			dat.DWord.H = h;
		}
	};

	CuCrypto_MakeU8ArrayHashImpl(Sha256::HashValueType);
}