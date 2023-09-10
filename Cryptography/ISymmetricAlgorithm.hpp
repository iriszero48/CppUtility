#pragma once

namespace CuCrypto
{
	template <typename T>
	class ISymmetricAlgorithm
	{
	public:
		decltype(auto) CreateEncryptor()
		{
			return static_cast<T *>(this)->CreateEncryptor();
		}

		decltype(auto) CreateDecryptor()
		{
			return static_cast<T *>(this)->CreateDecryptor();
		}
	};
}
