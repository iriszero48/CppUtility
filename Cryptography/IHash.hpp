#pragma once

#include "Utility.hpp"

namespace CuCrypto
{
	template <typename ValueType>
	class Hash
	{
	public:
		using HashValueType = ValueType;

		HashValueType Data{};

		Hash(const HashValueType &val);

		void ToString(std::string &buf) const;

		[[nodiscard]] std::string ToString() const
		{
			std::string buf{};
			ToString(buf);
			return buf;
		}

		operator std::string() const
		{
			return ToString();
		}

		bool operator==(const std::string &hashStr) const;

		bool operator!=(const std::string &hashStr) const
		{
			return !(*this == hashStr);
		}

		bool operator==(const Hash &hash) const;

		bool operator!=(const Hash &hash) const
		{
			return !(*this == hash);
		}
	};

#define CuCrypto_MakeU8ArrayHashImpl(type)                               \
	template <>                                                          \
	inline Hash<type>::Hash(const type &val) : Data(val)                 \
	{                                                                    \
	}                                                                    \
                                                                         \
	template <>                                                          \
	inline void Hash<type>::ToString(std::string &buf) const             \
	{                                                                    \
		Detail::Hash::Uint8ArrayToStringPaddingZero(Data, buf);          \
	}                                                                    \
                                                                         \
	template <>                                                          \
	inline bool Hash<type>::operator==(const std::string &hashStr) const \
	{                                                                    \
		return Detail::Hash::HashStrCmp(*this, hashStr);                 \
	}                                                                    \
                                                                         \
	template <>                                                          \
	inline bool Hash<type>::operator==(const Hash &hash) const           \
	{                                                                    \
		return Detail::Hash::ArrayCmp(Data, hash.Data);                  \
	}
}