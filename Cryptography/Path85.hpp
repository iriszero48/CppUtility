#pragma once

#include "Base85.hpp"

namespace CuCrypto
{
	namespace Detail::Path85
	{
		static constexpr std::array<char, 86> Table{
			R"(!#$%&'()+,-.0123456789;=@ABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_`abcdefghijklmnopqrstuvwxyz{}~)"};
	}

	using Path85 = Detail::Base85::Base85Base<Detail::Path85::Table, false>;
}