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
#endif

namespace CuExcept
{
	class DirtyUtf8Exception : public std::exception
	{
		std::u8string Msg{};
	public:
		DirtyUtf8Exception() = default;
		DirtyUtf8Exception(const std::u8string_view& msg) : Msg(msg) {}

		[[nodiscard]] char const* what() const override
		{
			return reinterpret_cast<const char*>(Msg.c_str());
		}

	};

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
		std::stacktrace_entry;
#else
		uint64_t;
#endif

#ifdef CU_EXCEPTION_USE_BOOST_STACKTRACE
#define CuExcept_GetStackTrace boost::stacktrace::stacktrace()
#elif defined(__cpp_lib_stacktrace)
#define CuExcept_GetStackTrace std::stacktrace_entry::current()
#else
#define CuExcept_GetStackTrace 0
#endif

		T Message{};

		std::source_location Source{};
		T ExceptionName{};

		StacktraceType StackTrace{};

		std::any InnerException{};

		std::any Data{};

		ExceptionBase(const std::exception& ex)
		{
			if constexpr (std::is_same_v<T, std::string>)
			{
				Message = ex.what();
				ExceptionName = typeid(ex).name();
			}
			else
			{
				Message = std::filesystem::path(ex.what()).string<typename T::value_type>();
				ExceptionName = std::filesystem::path(typeid(ex).name()).string<typename T::value_type>();
			}

			Data = ex;
		}

		template <typename Et>
		ExceptionBase(const ExceptionBase<Et>& ex):
			Message(std::filesystem::path(ex.Message).string<typename T::value_type>()), Source(ex.Source),
			ExceptionName(std::filesystem::path(ex.ExceptionName).string<typename T::value_type>()),
			StackTrace(ex.StackTrace), InnerException(ex.InnerException), Data(ex.Data)
		{
		}

		ExceptionBase(T message, T exceptionName = ToTString(GetExceptionName<ExceptionBase>{}()), std::source_location source = std::source_location::current(),
		          StacktraceType stackTrace = CuExcept_GetStackTrace,
		          std::any innerException = {}, std::any data = {}) : Message(std::move(message)),
			Source(std::move(source)),
			ExceptionName(std::move(exceptionName)),
			StackTrace(std::move(stackTrace)),
			InnerException(std::move(innerException)),
			Data(std::move(data))
		{
		}

		ExceptionBase() = default;

		operator std::exception() const
		{
			return static_cast<std::exception>(ToStdException());
		}

		[[nodiscard]] DirtyUtf8Exception ToStdException() const
		{
			if constexpr (std::is_same_v<T, std::u8string>)
			{
				return DirtyUtf8Exception(ToString());
			}
			else
			{
				return DirtyUtf8Exception(std::filesystem::path(ToString()).u8string());
			}
		}

		[[nodiscard]] char const* what() const override
		{
			return reinterpret_cast<const char*>(Message.c_str());
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
				std::stacktrace::to_string(st);
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
		ex(T message, T exceptionName = baseNamespace::base::ToTString(std::string_view(typeid(ex).name())), std::source_location source = std::source_location::current(),\
StacktraceType stackTrace = CuExcept_GetStackTrace,\
std::any innerException = {}, std::any data = {}) : baseNamespace::base(message,\
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
