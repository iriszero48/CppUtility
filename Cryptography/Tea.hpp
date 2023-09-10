#pragma once

#include "ITransform.hpp"
#include "ISymmetricAlgorithm.hpp"

#include "../Bit/Bit.hpp"

namespace CuCrypto
{
	namespace Detail::Tea
	{
		template <CuBit::Endian BitEndian>
		void Uint8ArrToUint32Arr(std::uint32_t out[2], const std::uint8_t in[8])
		{
			using namespace CuBit;

			out[0] = *reinterpret_cast<const uint32_t *>(&in[0 * 4]);
			out[1] = *reinterpret_cast<const uint32_t *>(&in[1 * 4]);

			if constexpr (BitEndian != Endian::Native)
			{
				out[0] = ByteSwap(out[0]);
				out[1] = ByteSwap(out[1]);
			}
		}

		template <CuBit::Endian BitEndian>
		void Uint32ArrToUint8Arr(std::uint8_t out[4], std::uint32_t in[2])
		{
			using namespace CuBit;

			auto a = in[0];
			auto b = in[1];

			if constexpr (BitEndian != Endian::Native)
			{
				a = ByteSwap(a);
				b = ByteSwap(b);
			}

			*reinterpret_cast<uint32_t *>(&out[0 * 4]) = a;
			*reinterpret_cast<uint32_t *>(&out[1 * 4]) = b;
		}
	}

	template <CuBit::Endian BitEndian = CuBit::Endian::Little>
	struct TeaKey
	{
		std::array<uint32_t, 4> Data{0, 0, 0, 0};

		TeaKey() = default;

		TeaKey(const std::uint8_t keyBytes[16])
		{
			using namespace CuBit;

			Data = {
				*(uint32_t *)&keyBytes[0 * 4],
				*(uint32_t *)&keyBytes[1 * 4],
				*(uint32_t *)&keyBytes[2 * 4],
				*(uint32_t *)&keyBytes[3 * 4]};

			if constexpr (BitEndian != Endian::Native)
			{
				Data[0] = ByteSwap(Data[0]);
				Data[1] = ByteSwap(Data[1]);
				Data[2] = ByteSwap(Data[2]);
				Data[3] = ByteSwap(Data[3]);
			}
		}

		uint32_t operator[](const size_t i) const
		{
			return Data[i];
		}
	};

	template <std::uint32_t Cycles = 32, CuBit::Endian BitEndian = CuBit::Endian::Little, std::uint32_t Delta = 0x9E3779B9>
	class Tea : public ISymmetricAlgorithm<Tea<Cycles, BitEndian, Delta>>
	{
	public:
		using KeyType = TeaKey<BitEndian>;

		class Encryptor : public ITransform<Encryptor, 8, 8, true, true>
		{
			KeyType key{};

		public:
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
				const auto [k0, k1, k2, k3] = key.Data;

				for (std::size_t i = 0; i < Cycles; i++)
				{
					v0 += (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + key[sum & 3]);
					sum += Delta;
					v1 += (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + key[(sum >> 11) & 3]);
				}

				data[0] = v0;
				data[1] = v1;
			}

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
				std::uint32_t sum = Delta * Cycles;

				for (std::size_t i = 0; i < Cycles; i++)
				{
					v1 -= (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + key[(sum >> 11) & 3]);
					sum -= Delta;
					v0 -= (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + key[sum & 3]);
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
