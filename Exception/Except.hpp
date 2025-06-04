#pragma once

#include <utility>
#include <any>
#include <string>
#include <string_view>

#include "../String/String.hpp"
#include "../Utility/Utility.hpp"

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

    using U8Str =
#ifdef __cpp_char8_t
            std::u8string;
#else
            std::string;
#endif

#define CuExcept_MakeExceptionName(type, name) \
	template <>                                \
	struct CuExcept::GetExceptionName<type>    \
	{                                          \
		std::string operator()() const         \
		{                                      \
			return name;                       \
		}                                      \
	};

	template <typename T>
	class ExceptionBase : public std::exception
	{
	public:
		using ValueType = T;

#define CuExcept_GetExceptionName(exceptionClass, prefix) prefix ToTString(CuExcept::GetExceptionName<exceptionClass>{}())

		T Message{};

		CuUtil::Source::SourceLocationType Source{};
		T ExceptionName{};

		CuUtil::Stacktrace::Stacktrace StackTrace{};

		std::any InnerException{};

		std::any Data{};

	private:
        U8Str whatBuff = u8"an Exception";

	public:
		ExceptionBase(T message, T exceptionName = CuExcept_GetExceptionName(ExceptionBase, ), CuUtil::Source::SourceLocationType source = CuUtil_Source_Current,
			CuUtil::Stacktrace::Stacktrace stackTrace = CuUtil::Stacktrace::Stacktrace::Current(),
					  std::any innerException = {}, std::any data = {}) : Message(std::move(message)),
																		  Source(std::move(source)),
																		  ExceptionName(std::move(exceptionName)),
																		  StackTrace(std::move(stackTrace)),
																		  InnerException(std::move(innerException)),
																		  Data(std::move(data))
		{
			FillWhat();
		}

		static ExceptionBase Create(const std::exception &ex)
		{
			return ExceptionBase(
				ToTString(std::string_view(ex.what())),
				CuExcept_GetExceptionName(decltype(ex), ),
				CuUtil_Source_Current,
				CuUtil::Stacktrace::Stacktrace::Current(), ex);
		}

		template <typename Et>
		static ExceptionBase Create(const ExceptionBase<Et> &ex)
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
			if constexpr (std::is_same_v<T, U8Str>)
			{
				whatBuff = std::move(msg);
			}
			else
			{
				whatBuff = (const U8Str::value_type *)std::filesystem::path(msg).u8string().c_str();
			}
		}

		[[nodiscard]] char const *what() const noexcept override
		{
			return reinterpret_cast<const char *>(whatBuff.c_str());
		}

		[[nodiscard]] virtual T ToString() const
		{
			using namespace std::string_view_literals;
			return Appends(
				ExceptionName, ": "sv, Message, "\n  at "sv,
				std::filesystem::path(Source.file_name()).filename().string<typename T::value_type>(), ":"sv, std::to_string(Source.line()),
				" "sv, std::string_view(Source.function_name()), "\n"sv, StackTrace.ToString());
		}

		template <typename Str>
		static T ToTString(Str &&str)
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

	private:
		template <typename Ct>
		static void Append(T &str, const std::basic_string_view<Ct> &data)
		{
			str.append(std::filesystem::path(data).string<typename T::value_type>());
		}

		static void Append(T &str, const std::basic_string_view<typename T::value_type> &data)
		{
			str.append(data);
		}

		template <typename Ct>
		static void Append(T &str, const std::basic_string<Ct> &data)
		{
			Append(str, std::basic_string_view<Ct>(data));
		}

		static void Append(T &str, const std::basic_string<typename T::value_type> &data)
		{
			str.append(data);
		}

		template <typename... Args>
		static T Appends(Args &&...args)
		{
			T str{};
			(Append(str, std::forward<Args>(args)), ...);
			return str;
		}
	};

	using Exception = ExceptionBase<std::string>;
	using U8Exception = ExceptionBase<U8Str>;
}

#define CuExcept_MakeException(ex, baseNamespace, base)                            \
	class ex : public baseNamespace::base                                          \
	{                                                                              \
	public:                                                                        \
		using T = baseNamespace::base::ValueType;                                  \
		ex(T message,                                                              \
		   T exceptionName = CuExcept_GetExceptionName(ex, baseNamespace::base::), \
		   CuUtil::Source::SourceLocationType source = CuUtil_Source_Current,                         \
		   CuUtil::Stacktrace::Stacktrace stackTrace = CuUtil::Stacktrace::Stacktrace::Current(),                     \
		   std::any innerException = {},                                           \
		   std::any data = {}) : baseNamespace::base(message,                      \
													 exceptionName,                \
													 source,                       \
													 stackTrace,                   \
													 innerException,               \
													 data)                         \
		{                                                                          \
		}                                                                          \
	};
#define CuExcept_MakeExceptionInstImpl(func, ex, ...) ex(func(__VA_ARGS__))

#define CuExcept_MakeExceptionInstFormat(ex, ...) CuExcept_MakeExceptionInstImpl(CuStr::Format, ex, __VA_ARGS__)
#define CuExcept_MakeExceptionInstFormatU8(ex, ...) CuExcept_MakeExceptionInstImpl(CuStr::FormatU8, ex, __VA_ARGS__)

#define CuExcept_MakeExceptionInstAppends(ex, ...) CuExcept_MakeExceptionInstImpl(CuStr::Appends, ex, __VA_ARGS__)
#define CuExcept_MakeExceptionInstAppendsU8(ex, ...) CuExcept_MakeExceptionInstImpl(CuStr::AppendsU8, ex, __VA_ARGS__)

#define CuExcept_MakeExceptionInstConstexprAppends(ex, ...) ex(CuUtil::String::Append(__VA_ARGS__).data())
#define CuExcept_MakeExceptionInstConstexprAppendsU8(ex, ...) CuExcept_MakeExceptionInstConstexprAppends(ex, __VA_ARGS__)

#define CuExcept_MakeExceptionInstCombine(ex, ...) CuExcept_MakeExceptionInstImpl(CuStr::Format, ex, __VA_ARGS__)
