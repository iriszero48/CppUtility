#pragma once

#include <string>
#include <vector>

namespace CuCrypto
{
	template <typename T>
	class IEncoding
	{
	public:
		template <typename SIt, typename DIt>
		void EncodeCore(SIt begin, SIt end, DIt dest)
		{
			static_cast<T *>(this)->EncodeCore(begin, end, dest);
		}

		template <typename SIt, typename DIt>
		void Encode(SIt begin, SIt end, DIt dest)
		{
			EncodeCore(begin, end, dest);
		}

		template <typename It>
		std::vector<char> Encode(It begin, It end)
		{
			std::vector<char> buf;
			Encode(begin, end, std::back_inserter(buf));
			return buf;
		}

		std::string Encode(const std::string_view &str)
		{
			std::string buf;
			Encode(str.begin(), str.end(), std::back_inserter(buf));
			return buf;
		}

		template <typename SIt, typename DIt>
		void DecodeCore(SIt begin, SIt end, DIt dest)
		{
			static_cast<T *>(this)->DecodeCore(begin, end, dest);
		}

		template <typename SIt, typename DIt>
		void Decode(SIt begin, SIt end, DIt dest)
		{
			DecodeCore(begin, end, dest);
		}

		template <typename It>
		std::vector<char> Decode(It begin, It end)
		{
			std::vector<char> buf;
			Decode(begin, end, std::back_inserter(buf));
			return buf;
		}

		std::string Decode(const std::string_view &str)
		{
			std::string buf;
			Decode(str.begin(), str.end(), std::back_inserter(buf));
			return buf;
		}
	};
}