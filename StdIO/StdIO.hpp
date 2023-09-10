#pragma once

#include <iostream>

#include "../Utility/Utility.hpp"

#ifdef CuUtil_Platform_Windows
#include <Windows.h>
#endif

namespace CuConsole
{
	enum class Color
	{
		Red,
		Yellow,
		White,
		Blue,
		Gray
	};

	namespace Detail
	{
		static Color ForegroundColor = Color::White;

		template <bool Line, typename Stream, typename... Args>
		void StreamCombine(Stream &stream, Args &&...args)
		{
			if constexpr (!Line)
				(stream << ... << args);
			if constexpr (Line)
				(stream << ... << args) << std::endl;
		}

		template <bool Line, typename Stream, typename... Args>
		void WriteImpl(Stream &stream, Args &&...args)
		{
#ifdef CuUtil_Platform_Windows
			static std::unordered_map<Color, WORD> ColorMap{
				{Color::Red, FOREGROUND_RED},
				{Color::Yellow, FOREGROUND_RED | FOREGROUND_GREEN},
				{Color::White, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN},
				{Color::Blue, FOREGROUND_BLUE},
				{Color::Gray, 0},
			};

			auto *const console = GetStdHandle(STD_OUTPUT_HANDLE);
			SetConsoleTextAttribute(console, ColorMap.at(ForegroundColor) | FOREGROUND_INTENSITY);
			StreamCombine<Line>(stream, std::forward<Args>(args)...);
#else
			if (ForegroundColor == Color::White)
			{
				StreamCombine<Line>(stream, std::forward<Args>(args)...);
				return;
			}

			static std::unordered_map<Color, std::string_view> colorMap{
				{Color::Red, "91"},
				{Color::Yellow, "93"},
				{Color::Blue, "94"},
				{Color::Gray, "90"},
			};

			StreamCombine<Line>(stream, "\x1B[", colorMap.at(ForegroundColor), "m", std::forward<Args>(args)..., "\033[0m");
#endif
		}
	}

	inline void SetForegroundColor(const Color &color)
	{
		Detail::ForegroundColor = color;
	}

	template <typename... Args>
	void Write(Args &&...args)
	{
		Detail::WriteImpl<false>(std::cout, std::forward<Args>(args)...);
	}

	template <typename... Args>
	void WriteLine(Args &&...args)
	{
		Detail::WriteImpl<true>(std::cout, std::forward<Args>(args)...);
	}

	namespace Error
	{
		template <typename... Args>
		void Write(Args &&...args)
		{
			Detail::WriteImpl<false>(std::cerr, std::forward<Args>(args)...);
		}

		template <typename... Args>
		void WriteLine(Args &&...args)
		{
			Detail::WriteImpl<true>(std::cerr, std::forward<Args>(args)...);
		}
	}
}
