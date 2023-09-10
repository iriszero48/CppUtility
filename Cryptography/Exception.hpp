#pragma once

#include "../Exception/Except.hpp"

namespace CuCrypto
{
	class Exception : std::exception
	{
		using std::exception::exception;
	};

#define CuCrypto_MakeExceptImpl(ex, ...) CuExcept_MakeStaticException(ex, __VA_ARGS__)
#define CuCrypto_MakeExcept(...) CuExcept_MakeStaticException(Exception, __VA_ARGS__)
#define CuCrypto_MakeDynExceptImpl(ex, ...) CuExcept_MakeException(ex, __VA_ARGS__)
}