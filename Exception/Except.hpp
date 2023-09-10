#pragma once

#include <utility>

#include "../String/String.hpp"
#include "../Utility/Utility.hpp"

#define CuExcept_MakeException(except, ...)                          \
	except(                                                          \
		CuStr::Combine(                                              \
			CuUtil::String::Combine(                                 \
				"[", CuUtil_Filename, ":", CuUtil_LineString, "] [", \
				__FUNCTION__, "] "                                   \
							  "[" #except "] ")                      \
				.data(),                                             \
			__VA_ARGS__))

#define CuExcept_MakeStaticException(except, ...)                \
	except(                                                      \
		CuUtil::String::Combine(                                 \
			"[", CuUtil_Filename, ":", CuUtil_LineString, "] [", \
			__FUNCTION__, "] "                                   \
						  "[" #except "] ",                      \
			__VA_ARGS__)                                         \
			.data())

namespace CuExcept
{
	class Exception : std::exception
	{
	public:
		std::string File;
		decltype(__LINE__) Line;
		std::string Function;
		std::string ExceptionName;

		Exception() = delete;

		template <typename WhatStr>
		Exception(std::string file, const decltype(Line) line, std::string function, std::string exception, WhatStr&& str) :
			std::exception(std::forward<WhatStr>(str)),
			File(std::move(file)), Line(line), Function(std::move(function)), ExceptionName(std::move(exception))
		{
		}
	};
}
