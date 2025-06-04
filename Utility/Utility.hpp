#pragma once

#include <array>
#include <utility>

#ifdef __cpp_lib_source_location
#define CU_UTILITY_USE_STD_SOURCE_LOCATION
#endif

#ifdef CU_UTILITY_USE_BOOST_STACKTRACE
#include <boost/stacktrace.hpp>
#elif defined(__cpp_lib_stacktrace)
#define CU_UTILITY_USE_STD_STACKTRACE
#include <stacktrace>
#endif

#ifdef CU_UTILITY_USE_STD_SOURCE_LOCATION
#include <source_location>
#endif

#pragma region private

#define CuUtil__ToStringFunc(x) #x

#pragma endregion private

#pragma region public

#define CuUtil_ToString(x) CuUtil__ToStringFunc(x)
#define CuUtil_LineString CuUtil_ToString(__LINE__)

namespace CuUtil
{
	template <typename T>
	class Range
	{
		T begVal = 0;
		T endVal;

	public:
		class Iterator
		{
			T val;

		public:
			using iterator_category = std::random_access_iterator_tag;
			using value_type = T;
			using difference_type = std::int64_t;
			using pointer = const T*;
			using reference = const T&;

			Iterator() : val(0) {}
			Iterator(T val) : val(val) {}

			Iterator& operator++()
			{
				++val;
				return *this;
			}

			bool operator==(Iterator other) const { return val == other.val; }
			bool operator!=(Iterator other) const { return !(*this == other); }
			bool operator<(Iterator other) const { return val < other.val; }

			reference operator*() const { return val; }
			value_type operator+(Iterator other) const { return val + other.val; }
			difference_type operator-(Iterator other) const { return val - other.val; }
		};

		Range(T count) : endVal(count) {}
		Range(T start, T count) : begVal(start), endVal(start + count) {}

		Iterator begin() { return Iterator(begVal); }
		Iterator end() { return Iterator(endVal); }
	};

	namespace Variant
	{
		template <typename... Func>
		struct Visitor : Func...
		{
			using Func::operator()...;
		};
		template <typename... Func>
		Visitor(Func...) -> Visitor<Func...>;
	}

	namespace String
	{
		template <typename>
		struct Length
		{
		};
		template <typename T, std::size_t S>
		struct Length<std::array<T, S>>
		{
			constexpr static size_t Value() { return S - 1; }
		};
		template <typename T, std::size_t S>
		struct Length<const std::array<T, S>>
		{
			constexpr static size_t Value() { return S - 1; }
		};
		template <typename T, std::size_t S>
		struct Length<const std::array<T, S> &>
		{
			constexpr static size_t Value() { return S - 1; }
		};
		template <typename T, std::size_t S>
		struct Length<std::array<T, S> &&>
		{
			constexpr static size_t Value() { return S - 1; }
		};
		template <typename T, std::size_t S>
		struct Length<const T (&)[S]>
		{
			constexpr static size_t Value() { return S - 1; }
		};

		template <typename T, size_t S>
		constexpr std::array<T, S> ToBuffer(const T (&str)[S])
		{
			std::array<T, S> buf{};
			for (size_t i = 0; i < S; ++i)
			{
				buf[i] = str[i];
			}
			return buf;
		}

		template <typename T, size_t S>
		constexpr std::array<T, S> ToBuffer(std::array<T, S> str)
		{
			return str;
		}

		template <typename>
		struct GetCharType;
		template <typename T, size_t S>
		struct GetCharType<const T (&)[S]>
		{
			using Type = T;
		};
		template <typename T, size_t S>
		struct GetCharType<std::array<T, S>>
		{
			using Type = T;
		};
		template <typename T, size_t S>
		struct GetCharType<const std::array<T, S>>
		{
			using Type = T;
		};
		template <typename T, size_t S>
		struct GetCharType<const std::array<T, S> &>
		{
			using Type = T;
		};
		template <typename T, size_t S>
		struct GetCharType<std::array<T, S> &&>
		{
			using Type = T;
		};

		namespace Detail
		{
			template <size_t BufLength, typename T, size_t S>
			constexpr void CombineAppend(size_t &idx, std::array<char, BufLength> &buf, const std::array<T, S> &str)
			{
				constexpr auto len = S - 1;
				for (size_t i = 0; i < len; ++i)
				{
					buf[idx + i] = str[i];
				}
				idx += len;
			}
		}

#ifdef __cpp_consteval
#define CuUtil_CONSTEVAL consteval
#else
#define CuUtil_CONSTEVAL constexpr
#endif

		template <typename... Args>
        CuUtil_CONSTEVAL auto Combine(Args &&...args)
		{
			constexpr auto length = (Length<Args>::Value() + ...);

			std::array<typename GetCharType<decltype(std::get<0>(std::forward_as_tuple(std::forward<Args>(args)...)))>::Type, length + 1> buf{};

			size_t i = 0;
			(Detail::CombineAppend(i, buf, ToBuffer(std::forward<Args>(args))), ...);
			buf[length] = 0;
			return buf;
		}

		namespace Detail
		{
			template <size_t BufLength, typename DT, size_t DS, typename ST, size_t SS>
			constexpr void JoinAppend(size_t& idx, std::array<char, BufLength>& buf, const std::array<DT, DS>& de, const std::array<ST, SS>& str)
			{
				if (idx != 0)
				{
					constexpr auto len = DS - 1;
					for (size_t i = 0; i < len; ++i)
					{
						buf[idx + i] = de[i];
					}
					idx += len;
				}

				constexpr auto len = SS - 1;
				for (size_t i = 0; i < len; ++i)
				{
					buf[idx + i] = str[i];
				}
				idx += len;
			}
		}

		template <typename Str, typename... Args>
		CuUtil_CONSTEVAL auto Join(Str&& str, Args &&...args)
		{
			constexpr auto length = (Length<Args>::Value() + ...) + ((sizeof...(Args)) - 1) * Length<Str>::Value();

			std::array<typename GetCharType<decltype(std::get<0>(std::forward_as_tuple(std::forward<Args>(args)...)))>::Type, length + 1> buf{};

			const auto de = ToBuffer(std::forward<Str>(str));
			size_t i = 0;
			(Detail::JoinAppend(i, buf, de, ToBuffer(std::forward<Args>(args))), ...);
			buf[length] = 0;
			return buf;
		}

		template <size_t N, typename T>
		CuUtil_CONSTEVAL auto Repeat(T &&str)
		{
			constexpr auto length = N * Length<T>::Value();

			std::array<typename GetCharType<T>::Type, length + 1> buf{};

			size_t idx = 0;
			for (size_t i = 0; i < N; ++i)
			{
				Detail::CombineAppend(idx, buf, ToBuffer(std::forward<T>(str)));
			}

			buf[length] = 0;
			return buf;
		}

		template <typename T, size_t S>
		CuUtil_CONSTEVAL auto Reverse(const std::array<T, S> &str)
		{
			std::array<T, S> buf{};
			buf[S - 1] = 0;

			for (size_t i = 0; i != S - 1; ++i)
			{
				buf[S - 1 - 1 - i] = str[i];
			}

			return buf;
		}

		static constexpr auto Nop = static_cast<size_t>(-1);

		template <size_t Length, typename T, size_t S>
		constexpr auto SubString(const std::array<T, S> &str, const size_t begin)
		{
			std::array<T, Length + 1> buf{};

			size_t bufIdx = 0;
			for (size_t i = begin; i != begin + Length; ++i, ++bufIdx)
			{
				buf[bufIdx] = str[i];
			}
			buf[bufIdx] = 0;

			return buf;
		}

		namespace Detail
		{
			template <typename T, size_t S>
			constexpr size_t TrimEndImplGetEndPos(const std::array<T, S> &str, const size_t begin, const size_t end, const T ch)
			{
				for (int64_t i = static_cast<int64_t>(end) - 1; i >= static_cast<int64_t>(begin); --i)
				{
					if (ch != str[i])
					{
						return i + 1;
					}
				}

				return begin;
			}
		}

		template <typename T, size_t S>
		constexpr size_t TrimEndLength(const std::array<T, S> &str, const size_t begin, const size_t end, const T ch)
		{
			const auto subEnd = Detail::TrimEndImplGetEndPos(str, begin, end, ch);
			return subEnd - begin;
		}

		template <size_t ResultLength, typename T, size_t S>
		constexpr auto TrimEnd(const std::array<T, S> &str, const size_t begin, const size_t end, const T ch)
		{
			const auto subEnd = Detail::TrimEndImplGetEndPos(str, begin, end, ch);

			std::array<T, ResultLength + 1> buf{};

			size_t bufIdx = 0;
			for (size_t i = begin; i != subEnd; ++i, ++bufIdx)
			{
				buf[bufIdx] = str[i];
			}
			buf[bufIdx] = 0;

			return buf;
		}

		template <typename T, size_t S>
		constexpr size_t TrimEnd(const std::array<T, S> &str, const size_t begin, const size_t end, const T ch)
		{
			return Detail::TrimEndImplGetEndPos(str, begin, end, ch);
		}

		namespace Detail
		{
			template <typename T, size_t S>
			constexpr size_t TrimBeginImplGetBeginPos(const std::array<T, S> &str, const size_t begin, const size_t end, const T ch)
			{
				for (size_t i = begin; i != end; ++i)
				{
					if (ch != str[i])
					{
						return i;
					}
				}
				return ch;
			}
		}

		template <typename T, size_t S>
		constexpr size_t TrimBeginImplGetLen(const std::array<T, S> &str, const size_t begin, const size_t end, const T ch)
		{
			const auto subBegin = Detail::TrimBeginImplGetBeginPos(str, begin, end, ch);
			return end - subBegin;
		}

		template <size_t ResultLength, typename T, size_t S>
		constexpr auto TrimBegin(const std::array<T, S> &str, const size_t begin, const size_t end, const T ch)
		{
			constexpr auto subBegin = Detail::TrimBeginImplGetBeginPos(str, begin, end, ch);

			std::array<T, ResultLength + 1> buf{};

			size_t bufIdx = 0;
			for (size_t i = subBegin; i != end; ++i, ++bufIdx)
			{
				buf[bufIdx] = str[i];
			}
			buf[bufIdx] = 0;

			return buf;
		}

		template <typename T, size_t S>
		constexpr size_t TrimBegin(const std::array<T, S> &str, const size_t begin, const size_t end, const T ch)
		{
			return Detail::TrimBeginImplGetBeginPos(str, begin, end, ch);
		}

		struct SubRange
		{
			size_t Begin;
			size_t End;
		};

		template <typename T, size_t S>
		constexpr auto Trim(const std::array<T, S> &str, const size_t begin, const size_t end, const T ch)
		{
			const auto subBegin = TrimBegin(str, begin, end, ch);
			const auto subEnd = TrimEnd(str, subBegin, end, ch);
			return SubRange{subBegin, subEnd};
		}

		template <typename T, size_t S>
		constexpr size_t TrimLength(const std::array<T, S> &str, const size_t begin, const size_t end, const T ch)
		{
			const auto rng = Trim(str, begin, end, ch);
			return rng.End - rng.Begin;
		}

		template <size_t ResultLength, typename T, size_t S>
		constexpr auto Trim(const std::array<T, S> &str, const size_t begin, const size_t end, const T ch)
		{
			const auto rng = Trim(str, begin, end, ch);

			return SubString<ResultLength>(str, rng.Begin);
		}
	}

	template <typename T, typename... Types>
	inline constexpr bool IsAnyOfV = std::disjunction_v<std::is_same<T, Types>...>;

	template <typename T, std::enable_if_t<std::is_enum_v<T>, bool> = true>
	std::underlying_type_t<T> ToUnderlying(T v)
	{
		return static_cast<std::underlying_type_t<T>>(v);
	}

	template <typename Ret, typename It>
	Ret CopyN(It&& begin, const size_t n)
	{
		Ret ret{};
		std::copy_n(begin, n, std::begin(ret));
		return ret;
	}

	namespace Convert
	{
		template <typename T, typename Ct, size_t Cs, std::enable_if_t<IsAnyOfV<T, uint8_t, uint16_t, uint32_t, uint64_t>, bool> = true>
		constexpr T ToIntegral(const std::array<Ct, Cs> &str, size_t begin, const size_t end, const int base = 10)
		{
			const auto len = end - begin;

			if (base == 10)
			{
				T val = 0;
				for (auto i = static_cast<int64_t>(begin); i < static_cast<int64_t>(end); ++i)
				{
					val *= 10;
					val += str[i] - '0';
				}
				return val;
			}
			else if (base == 16)
			{
				if (len > 2 && str[begin] == '0' && (str[begin + 1] == 'x' || str[begin + 1] == 'X'))
					begin += 2;

				T val = 0;
				for (auto i = static_cast<int64_t>(begin); i < static_cast<int64_t>(end); ++i)
				{
					val *= 16;
					if (const auto ch = str[i]; ch >= 'a' && ch <= 'f')
					{
						val += ch - 'a' + 10;
					}
					else if (ch >= 'A' && ch <= 'F')
					{
						val += ch - 'A' + 10;
					}
					else
					{
						val += ch - '0';
					}
				}
				return val;
			}

			return -1;
		}

		template <typename T, typename Ct, size_t Cs, std::enable_if_t<IsAnyOfV<T, int8_t, int16_t, int32_t, int64_t>, bool> = true>
		constexpr T ToIntegral(const std::array<Ct, Cs> &str, size_t begin, const size_t end, const int base = 10)
		{
			const auto len = end - begin;

			if (base == 10)
			{
				T val = 0;
				bool negFlag = str[begin] == '-';
				if (str[begin] == '-' || str[begin] == '+')
					begin++;

				for (auto i = static_cast<int64_t>(begin); i != static_cast<int64_t>(end); ++i)
				{
					val *= 10;
					val += str[i] - '0';
				}
				return negFlag ? -val : val;
			}
			else if (base == 16)
			{
				return static_cast<T>(ToIntegral<std::make_unsigned_t<T>>(str, begin, end, base));
			}

			return -1;
		}
	}

	namespace Source
	{
		namespace Detail
		{
			template <typename T, size_t S>
			CuUtil_CONSTEVAL size_t GetFilenameOffset(const T (&str)[S])
			{
				for (size_t i = S - 1; i > 0; --i)
				{
					if (str[i] == '/' || str[i] == '\\')
					{
						return i + 1;
					}
				}

				return 0;
			}

			template <size_t Offset, typename T, size_t S>
			CuUtil_CONSTEVAL auto GetFilenameImpl(const T (&str)[S])
			{
				std::array<T, S - Offset> buf{};
				for (size_t i = 0; i < buf.size(); ++i)
				{
					buf[i] = str[Offset + i];
				}
				return buf;
			}
		}

#ifndef CU_UTILITY_USE_STD_SOURCE_LOCATION
		struct SourceLocation {
			constexpr SourceLocation() = default;

			constexpr SourceLocation(const std::uint_least32_t line,
				const char* const file,
				const char* const function = "<unknown function>") noexcept : line_(line), file_(file), function_(function) {}

			constexpr uint_least32_t line() const noexcept {
				return line_;
			}
			constexpr uint_least32_t column() const noexcept {
				return 0;
			}
			constexpr const char* file_name() const noexcept {
				return file_;
			}
			constexpr const char* function_name() const noexcept {
				return function_;
			}

		private:
			std::uint_least32_t line_{};
			const char* file_ = "";
			const char* function_ = "";
		};
#endif

		using SourceLocationType =
#ifdef CU_UTILITY_USE_STD_SOURCE_LOCATION
			std::source_location;
#else
			SourceLocation;
#endif

#ifdef CU_UTILITY_USE_STD_SOURCE_LOCATION
#define CuUtil_Source_Current std::source_location::current()
#else
#define CuUtil_Source_Current CuUtil::Source::SourceLocation(__LINE__, __FILE__)
#endif

#define CuUtil_GetFilename(path) CuUtil::Source::Detail::GetFilenameImpl<CuUtil::Source::Detail::GetFilenameOffset(path)>(path)
#define CuUtil_Filename CuUtil_GetFilename(__FILE__)
	}

	namespace Stacktrace
	{
		struct Stacktrace
		{
#ifdef CU_UTILITY_USE_BOOST_STACKTRACE
			boost::stacktrace::stacktrace Native{};
#elif defined(CU_UTILITY_USE_STD_STACKTRACE)
			decltype(std::stacktrace::current())  Native{};
#else
			static constexpr const char* Native = "<unknown stack>";
#endif

			static Stacktrace Current()
			{
				Stacktrace st{};
#ifdef CU_UTILITY_USE_BOOST_STACKTRACE
				st.Native = boost::stacktrace::stacktrace();
#elif defined(CU_UTILITY_USE_STD_STACKTRACE)
				st.Native = std::stacktrace::current();
#endif
				return st;
			}

			std::string ToString() const
			{
				return
#ifdef CU_UTILITY_USE_BOOST_STACKTRACE
					boost::stacktrace::to_string(Native);
#elif defined(CU_UTILITY_USE_STD_STACKTRACE)
					std::to_string(Native);
#else
					Native;
#endif		
			}
		};
	}

#if defined(DEBUG) || defined(_DEBUG)
#define CuUtil_Assert_DebugOnly(expr, ex) CuUtil_Assert(expr, ex)
#else
#define CuUtil_Assert_DebugOnly(expr, ex)
#endif

	enum class Platform
	{
		Other = 0,
		Windows = 1,
		Linux = 2,
		Macintosh = 3,
		Native =
#if __linux__
			Linux
#define CuUtil_Platform_Linux
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__) || defined(_WIN64)
			Windows
#define CuUtil_Platform_Windows
#elif __APPLE__
			Macintosh
#define CuUtil_Platform_Macintosh
#else
			Other
#define CuUtil_Platform_Other
#endif
	};

	enum class Compiler
	{
		Other = 0,
		MSVC = 1,
		GCC = 2,
		CLANG = 3,
		Native =
#if defined(__clang__)
			CLANG
#define CuUtil_Compiler_CLANG
#elif defined(__GNUC__) || defined(__GNUG__)
			GCC
#define CuUtil_Compiler_GCC
#elif defined(_MSC_VER)
			MSVC
#define CuUtil_Compiler_MSVC
#else
			Other
#define CuUtil_Compiler_Other
#endif
	};
}

#pragma endregion public
