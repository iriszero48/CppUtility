#pragma once

#include "../Exception/Except.hpp"
#include "../Utility/Utility.hpp"
#include "../String/String.hpp"

namespace CuCrypto
{
	CuExcept_MakeException(Exception, CuExcept, Exception);

#define CuCrypto_MakeExceptImpl(ex, ...) ex(CuUtil::String::Combine(__VA_ARGS__).data())
#define CuCrypto_MakeExcept(...) CuCrypto_MakeExceptImpl(Exception, __VA_ARGS__)
#define CuCrypto_MakeDynExceptImpl(ex, ...) ex(CuStr::Combine(__VA_ARGS__))
}