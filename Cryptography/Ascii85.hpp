#pragma once

#include "Base85.hpp"

namespace CuCrypto
{
	namespace Detail::Ascii85
	{
		static constexpr std::array<char, 86> Table{
			R"(!"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstu)"};
	}

	using Ascii85 = Detail::Base85::Base85Base<Detail::Ascii85::Table, true>;
}