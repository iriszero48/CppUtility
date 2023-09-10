#pragma once

#include "Tea.hpp"

namespace CuCrypto
{
	template <std::uint32_t Cycles = 32, CuBit::Endian BitEndian = CuBit::Endian::Little, std::uint32_t Delta = 0x9E3779B9>
	class XTea : public ISymmetricAlgorithm<XTea<Cycles, BitEndian, Delta>>
	{
	public:
		using KeyType = TeaKey<BitEndian>;

		class Encryptor : public ITransform<Encryptor, 8, 8, true, true>
		{
			KeyType key{};

			void Encrypt8(const std::uint8_t *input, std::uint8_t *output, const std::size_t round) const
			{
				for (std::size_t i = 0; i < round; ++i)
				{
					const auto *dataIn = input + i * 8;
					auto *dataOut = output + i * 8;

					std::uint32_t dat[2];
					Detail::Tea::Uint8ArrToUint32Arr<BitEndian>(dat, dataIn);
					Encrypt8Impl(dat);
					Detail::Tea::Uint32ArrToUint8Arr<BitEndian>(dataOut, dat);
				}
			}

			void Encrypt8Impl(std::uint32_t data[2]) const
			{
				std::uint32_t v0 = data[0];
				std::uint32_t v1 = data[1];
				std::uint32_t sum = 0;
				const auto &[k0, k1, k2, k3] = key.Data;

				for (std::uint32_t i = 0; i < Cycles; i++)
				{
					sum += Delta;
					v0 += ((v1 << 4) + k0) ^ (v1 + sum) ^ ((v1 >> 5) + k1);
					v1 += ((v0 << 4) + k2) ^ (v0 + sum) ^ ((v0 >> 5) + k3);
				}

				data[0] = v0;
				data[1] = v1;
			}

		public:
			explicit Encryptor(KeyType key) : key(key) {}

			std::size_t TransformCore(const std::uint8_t *input, std::size_t size, std::uint8_t *output) const
			{
				const auto n = size / this->InputBlockSize;
				Encrypt8(input, output, n);

				return n * this->OutputBlockSize;
			}
		};

		class Decryptor : public ITransform<Decryptor, 8, 8, true, true>
		{
			KeyType key{};

			void Decrypt8(const std::uint8_t *input, std::uint8_t *output, const std::size_t round) const
			{
				for (std::size_t i = 0; i < round; ++i)
				{
					const auto *dataIn = input + i * 8;
					auto *dataOut = output + i * 8;

					std::uint32_t dat[2];
					Detail::Tea::Uint8ArrToUint32Arr<BitEndian>(dat, dataIn);
					Decrypt8Impl(dat);
					Detail::Tea::Uint32ArrToUint8Arr<BitEndian>(dataOut, dat);
				}
			}

			void Decrypt8Impl(std::uint32_t data[2]) const
			{
				std::uint32_t v0 = data[0];
				std::uint32_t v1 = data[1];
				std::uint32_t sum = (Delta * Cycles) & static_cast<std::uint32_t>(-1);
				const auto &[k0, k1, k2, k3] = key.Data;

				for (std::uint32_t i = 0; i < Cycles; i++)
				{
					v1 -= ((v0 << 4) + k2) ^ (v0 + sum) ^ ((v0 >> 5) + k3);
					v0 -= ((v1 << 4) + k0) ^ (v1 + sum) ^ ((v1 >> 5) + k1);
					sum -= Delta;
				}
				data[0] = v0;
				data[1] = v1;
			}

		public:
			explicit Decryptor(KeyType key) : key(key) {}

			std::size_t TransformCore(const std::uint8_t *input, std::size_t size, std::uint8_t *output) const
			{
				const auto n = size / this->InputBlockSize;
				Decrypt8(input, output, n);

				return this->OutputBlockSize * n;
			}
		};

		KeyType Key{};

		Encryptor CreateEncryptor() const { return Encryptor(Key); }
		Decryptor CreateDecryptor() const { return Decryptor(Key); }
	};
}
