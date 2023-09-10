#pragma once

#include "Tea.hpp"

namespace CuCrypto
{
	namespace Detail::XXTea
	{
		template <CuBit::Endian BitEndian>
		void BSwapUint32Arr(std::uint32_t *v, const std::size_t n)
		{
			using namespace CuBit;

			if constexpr (BitEndian != Endian::Native)
			{
				std::transform(v, v + n, v, ByteSwap<std::uint32_t>);
			}
		}
	}

	template <CuBit::Endian BitEndian = CuBit::Endian::Little, std::uint32_t Delta = 0x9E3779B9>
	class XXTea : public ISymmetricAlgorithm<XXTea<BitEndian, Delta>>
	{
	public:
		using KeyType = TeaKey<BitEndian>;

		class Encryptor : public ITransform<Encryptor, 4, 4, false, true>
		{
			KeyType key{};

		public:
			Encryptor(KeyType key) : key(key) {}

#define MX (((z >> 5 ^ y << 2) + (y >> 3 ^ z << 4)) ^ ((sum ^ y) + (key[(p & 3) ^ e] ^ z)))
			std::size_t TransformCore(const std::uint8_t *input, std::size_t size, std::uint8_t *output) const
			{
				std::copy_n(input, size, output);
				auto *v = reinterpret_cast<std::uint32_t *>(output);
				const auto n = size / this->InputBlockSize;
				Detail::XXTea::BSwapUint32Arr<BitEndian>(v, n);

				auto rounds = 6 + 52 / n;
				std::uint32_t sum = 0;
				auto z = v[n - 1];
				do
				{
					sum += Delta;
					const auto e = (sum >> 2) & 3;
					std::uint32_t y;
					std::size_t p;
					for (p = 0; p < n - 1; p++)
					{
						y = v[p + 1];
						z = v[p] += MX;
					}
					y = v[0];
					z = v[n - 1] += MX;
				} while (--rounds);

				Detail::XXTea::BSwapUint32Arr<BitEndian>(v, n);

				return n * this->OutputBlockSize;
			}
		};

		class Decryptor : public ITransform<Decryptor, 4, 4, false, true>
		{
			KeyType key{};

		public:
			Decryptor(KeyType key) : key(key) {}

			std::size_t TransformCore(const std::uint8_t *input, std::size_t size, std::uint8_t *output) const
			{
				std::copy_n(input, size, output);
				auto *v = reinterpret_cast<std::uint32_t *>(output);
				const auto n = size / 4;
				Detail::XXTea::BSwapUint32Arr<BitEndian>(v, n);

				auto rounds = 6 + 52 / n;
				auto sum = rounds * Delta;
				auto y = v[0];
				do
				{
					const auto e = (sum >> 2) & 3;
					std::uint32_t z;
					std::size_t p;
					for (p = n - 1; p > 0; p--)
					{
						z = v[p - 1];
						y = v[p] -= MX;
					}
					z = v[n - 1];
					y = v[0] -= MX;
					sum -= Delta;
				} while (--rounds);

				Detail::XXTea::BSwapUint32Arr<BitEndian>(v, n);

				return this->OutputBlockSize * n;
			}
#undef MX
		};

		KeyType Key{};

		Encryptor CreateEncryptor() const { return Encryptor(Key); }
		Decryptor CreateDecryptor() const { return Decryptor(Key); }
	};
}