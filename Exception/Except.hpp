#pragma once

#include <utility>
#include <source_location>
#include <any>
#include <string>
#include <string_view>

#include "../String/String.hpp"
#include "../Utility/Utility.hpp"

#ifdef CU_EXCEPTION_USE_BOOST_STACKTRACE
#include <boost/stacktrace.hpp>
#elif defined(__cpp_lib_stacktrace)
#include <stacktrace>
#endif

namespace CuExcept
{
	template <typename Except>
	struct GetExceptionName
	{
		std::string operator()() const
		{
			return typeid(Except).name();
		}
	};

#define CuExcept_MakeExceptionName(type, name)\
	template <>\
	struct CuExcept::GetExceptionName<type>\
	{\
		std::string operator()() const\
		{\
			return name;\
		}\
	};

	template <typename T>
	class ExceptionBase : public std::exception
	{
	public:
		using ValueType = T;

		using StacktraceType =
#ifdef CU_EXCEPTION_USE_BOOST_STACKTRACE
		boost::stacktrace::stacktrace;
#elif defined(__cpp_lib_stacktrace)
		decltype(std::stacktrace::current());
#else
		uint64_t;
#endif

#ifdef CU_EXCEPTION_USE_BOOST_STACKTRACE
#define CuExcept_GetStackTrace boost::stacktrace::stacktrace()
#elif defined(__cpp_lib_stacktrace)
#define CuExcept_GetStackTrace std::stacktrace::current()
#else
#define CuExcept_GetStackTrace 0
#endif

#define CuExcept_GetExceptionName(exceptionClass, prefix) prefix ToTString(CuExcept::GetExceptionName<exceptionClass>{}())

#define CuExcept_GetSource std::source_location::current()

		T Message{};

		std::source_location Source{};
		T ExceptionName{};

		StacktraceType StackTrace{};

		std::any InnerException{};

		std::any Data{};

	private:
		std::u8string whatBuff = u8"an Exception";
		
	public:
		ExceptionBase(T message, T exceptionName = CuExcept_GetExceptionName(ExceptionBase, ), std::source_location source = CuExcept_GetSource,
			StacktraceType stackTrace = CuExcept_GetStackTrace,
			std::any innerException = {}, std::any data = {}) : Message(std::move(message)),
			Source(std::move(source)),
			ExceptionName(std::move(exceptionName)),
			StackTrace(std::move(stackTrace)),
			InnerException(std::move(innerException)),
			Data(std::move(data))
		{
			FillWhat();
		}

		static ExceptionBase Create(const std::exception& ex)
		{
			return ExceptionBase(
				ToTString(std::string_view(ex.what())),
				CuExcept_GetExceptionName(decltype(ex), ),
				CuExcept_GetSource,
				CuExcept_GetStackTrace, ex);
		}

		template <typename Et>
		static ExceptionBase Create(const ExceptionBase<Et>& ex)
		{
			return ExceptionBase(
				ToTString(ex.Message),
				ToTString(ex.ExceptionName),
				ex.Source,
				ex.StackTrace);
		}

		void FillWhat()
		{
			auto msg = ToString();
			if constexpr (std::is_same_v<T, std::u8string>)
			{
				whatBuff = std::move(msg);
			}
			else
			{
				whatBuff = std::filesystem::path(msg).u8string();
			}
		}

		[[nodiscard]] char const* what() const override
		{
			return reinterpret_cast<const char*>(whatBuff.c_str());
		}

		[[nodiscard]] virtual T ToString() const
		{
			using namespace std::string_view_literals;
			return Appends(
				ExceptionName, ": "sv, Message, "\n  at "sv,
				std::filesystem::path(Source.file_name()).filename().string<typename T::value_type>(), ":"sv, std::to_string(Source.line()),
				" "sv, std::string_view(Source.function_name()), "\n"sv, StackTraceToString(StackTrace));
		}

		template <typename Str>
		static T ToTString(Str&& str)
		{
			if constexpr (std::is_same_v<typename T::value_type, typename Str::value_type>)
			{
				return T(str);
			}
			else
			{
				return std::filesystem::path(std::forward<Str>(str)).string<typename T::value_type>();
			}
		}

		static std::string StackTraceToString(decltype(StackTrace) const& st)
		{
			return
#ifdef CU_EXCEPTION_USE_BOOST_STACKTRACE
				boost::stacktrace::to_string(st);
#elif defined(__cpp_lib_stacktrace)
				std::to_string(st);
#else
				std::to_string(st);
#endif
		}

	private:
		template <typename Ct>
		static void Append(T& str, const std::basic_string_view<Ct>& data)
		{
			str.append(std::filesystem::path(data).string<typename T::value_type>());
		}

		static void Append(T& str, const std::basic_string_view<typename T::value_type>& data)
		{
			str.append(data);
		}

		template <typename Ct>
		static void Append(T& str, const std::basic_string<Ct>& data)
		{
			Append(str, std::basic_string_view<Ct>(data));
		}

		static void Append(T& str, const std::basic_string<typename T::value_type>& data)
		{
			str.append(data);
		}

		template <typename... Args>
		static T Appends(Args&& ...args)
		{
			T str{};
			(Append(str, std::forward<Args>(args)), ...);
			return str;
		}
	};

	using Exception = ExceptionBase<std::string>;
	using U8Exception = ExceptionBase<std::u8string>;
}

#define CuExcept_MakeException(ex, baseNamespace, base)\
	class ex : public baseNamespace::base\
	{\
	public:\
		using T = baseNamespace::base::ValueType;\
		ex(T message,\
			T exceptionName = CuExcept_GetExceptionName(ex, baseNamespace::base::),\
			std::source_location source = CuExcept_GetSource,\
			StacktraceType stackTrace = CuExcept_GetStackTrace,\
			std::any innerException = {},\
			std::any data = {}) : baseNamespace::base(message,\
exceptionName,\
source,\
stackTrace,\
innerException,\
data)\
{\
}\
	};
#define CuExcept_MakeExceptionInstImpl(func, ex, ...) ex(func(__VA_ARGS__))

#define CuExcept_MakeExceptionInstFormat(ex, ...) CuExcept_MakeExceptionInstImpl(CuStr::Format, ex, __VA_ARGS__)
#define CuExcept_MakeExceptionInstFormatU8(ex, ...) CuExcept_MakeExceptionInstImpl(CuStr::FormatU8, ex, __VA_ARGS__)

#define CuExcept_MakeExceptionInstAppends(ex, ...) CuExcept_MakeExceptionInstImpl(CuStr::Appends, ex, __VA_ARGS__)
#define CuExcept_MakeExceptionInstAppendsU8(ex, ...) CuExcept_MakeExceptionInstImpl(CuStr::AppendsU8, ex, __VA_ARGS__)

#define CuExcept_MakeExceptionInstConstexprAppends(ex, ...) ex(CuUtil::String::Append(__VA_ARGS__).data())
#define CuExcept_MakeExceptionInstConstexprAppendsU8(ex, ...) CuExcept_MakeExceptionInstConstexprAppends(ex, __VA_ARGS__)

#define CuExcept_MakeExceptionInstCombine(ex, ...) CuExcept_MakeExceptionInstImpl(CuStr::Format, ex, __VA_ARGS__)
