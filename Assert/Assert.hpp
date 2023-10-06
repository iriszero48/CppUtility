#pragma once

#include "../Utility/Utility.hpp"

namespace CuAssert
{
	CuExcept_MakeException(AssertError, CuExcept, Exception);

#define CuAssert_What(expr, what) if (!(expr)) throw CuAssert::AssertError(what)
#define CuAssert(expr) CuAssert_What(expr, (CuUtil::String::Combine("CuAssert(" #expr ") failed.").data()))
}
