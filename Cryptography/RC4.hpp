#pragma once

#include "ISymmetricAlgorithm.hpp"
#include "ITransform.hpp"

namespace CuCrypto
{
	struct RC4Key
	{
		std::array<std::uint8_t, 256> KeyBuf{};
		std::uint8_t KeyLength = 0;

		RC4Key() = default;
		RC4Key(const std::uint8_t *key, const std::size_t keyLength);
	};

	class RC4 : public ISymmetricAlgorithm<RC4>
	{
	public:
		class RC4Random
		{
			std::array<std::uint8_t, 256> s;
			std::int32_t i = 0;
			std::int32_t j = 0;

		public:
			RC4Random(const RC4Key &key);

			std::uint8_t Next();

			void Reset();
		};

		class Encryptor : public ITransform<Encryptor, 1, 1, true, true>
		{
		private:
			RC4Random rnd;

		public:
			Encryptor(const RC4Key &key) : rnd(key) {}

			std::size_t TransformCore(const std::uint8_t *input, std::size_t size, std::uint8_t *output);
		};

		class Decryptor : public ITransform<Decryptor, 1, 1, true, true>
		{
		private:
			RC4Random rnd;

		public:
			Decryptor(const RC4Key &key) : rnd(key) {}

			std::size_t TransformCore(const std::uint8_t *input, std::size_t size, std::uint8_t *output);
		};

	private:
		RC4Key key{};

	public:
		RC4() = delete;
		RC4(const std::uint8_t *key, const std::uint8_t keyLength) : key(key, keyLength) {}

		Encryptor CreateEncryptor() const { return Encryptor(key); }
		Decryptor CreateDecryptor() const { return Decryptor(key); }
	};
}
