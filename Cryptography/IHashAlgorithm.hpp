#pragma once

#include "IHash.hpp"
#include "Exception.hpp"

#include <fstream>

#ifdef __cpp_lib_span
#include <span>
#endif

namespace CuCrypto
{
	template <typename T, typename Type>
	class IHashAlgorithm
	{
	public:
		using HashValueType = Type;
		using HashType = Hash<HashValueType>;

		void AppendCore(const std::uint8_t *buf, const std::size_t len)
		{
			static_cast<T *>(this)->AppendCore(buf, len);
		}

		T &Append(const void *buf, const std::size_t len)
		{
			AppendCore(static_cast<const std::uint8_t *>(buf), len);
			return *static_cast<T *>(this);
		}

#ifdef __cpp_lib_span
		template <typename Dt, size_t S>
		T &Append(const std::span<Dt, S> &data)
		{
			return Append(data.data(), data.size_bytes());
		}
#endif

		T &Append(std::istream &stream)
		{
			if (!stream)
				throw CuCrypto_MakeExcept("bad stream");

			while (!stream.eof())
			{
				char buf[4096]{0};
				stream.read(buf, 4096);
				const auto count = stream.gcount();
				Append(reinterpret_cast<const uint8_t *>(buf), count);
			}

			return *static_cast<T *>(this);
		}

		template <typename Ch>
		T &Append(const std::basic_string_view<Ch> &str)
		{
			return Append(str.data(), str.length());
		}

		template <typename Ch>
		T &Append(const std::basic_string<Ch> &str)
		{
			return Append(str.data(), str.length());
		}

		template <typename Ch, std::size_t S, std::enable_if_t<CuStr::IsCharV<Ch>, bool> = false>
		T &Append(const Ch (&str)[S])
		{
			return Append(str, S - 1);
		}

		template <typename Dt, std::size_t S, std::enable_if_t<!CuStr::IsCharV<Dt>, bool> = false>
		T &Append(const Dt (&data)[S])
		{
			return Append(data, S);
		}

		T &Append(const std::filesystem::path &str)
		{
			std::ifstream fs(str, std::ios::in | std::ios::binary);
			Append(fs);
			fs.close();
			return *static_cast<T *>(this);
		}

		HashType Digest()
		{
			return static_cast<T *>(this)->Digest;
		}
	};
}