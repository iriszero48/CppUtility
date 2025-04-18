#pragma once

#include "Log.hpp"
#include "../Time/Time.hpp"
#include "../StdIO/StdIO.hpp"

#include <source_location>
#include <thread>

namespace CuLog
{
    struct LogMessage
    {
        std::chrono::system_clock::time_point Time;
		std::thread::id ThreadId;
        std::string Source;
        std::u8string Message;

        static std::string LogTime(const std::chrono::system_clock::time_point& time)
		{
			const auto t = std::chrono::system_clock::to_time_t(time);
			tm local{};
			CuTime::Local(&local, &t);
			std::ostringstream ss;
			ss << std::put_time(&local, "%F %X");
			return ss.str();
		}
    };

    static CuLog::Logger<LogMessage> Log{};
    static std::thread LogThread{};

	inline void LogHandler()
	{
		const std::unordered_map<CuLog::LogLevel, CuConsole::Color> colorMap
		{
			{CuLog::LogLevel::None, CuConsole::Color::White },
			{CuLog::LogLevel::Error, CuConsole::Color::Red },
			{CuLog::LogLevel::Warn, CuConsole::Color::Yellow },
			{CuLog::LogLevel::Info, CuConsole::Color::White },
			{CuLog::LogLevel::Verb, CuConsole::Color::Gray },
			{CuLog::LogLevel::Debug, CuConsole::Color::Blue }
		};

		while (true)
		{
			const auto [level, raw] = Log.Chan.Read();
			const auto& [time, thId, src, msg] = raw;

			auto out = CuStr::FormatU8("[{}] [{}] [{}] {}{}", CuEnum::ToString(level), LogMessage::LogTime(time), thId, src, msg);
			if (out[out.length() - 1] == u8'\n') out.erase(out.length() - 1);

			SetForegroundColor(colorMap.at(level));
			CuConsole::WriteLine(CuStr::ToDirtyUtf8StringView(out));

			if (level == CuLog::LogLevel::None) break;
		}
	}

#define LogImpl(level, ...)\
	if((int)level <= (int)CuLog::Log.Level) CuLog::Log.Write<level>(\
		std::chrono::system_clock::now(),\
		std::this_thread::get_id(),\
		std::string(CuUtil::String::Combine("[", CuUtil_Filename, ":", CuUtil_LineString "] [", __FUNCTION__ , "] ").data()),\
		CuStr::FormatU8(__VA_ARGS__))

    void Init()
    {
        LogThread = std::thread(LogHandler);
    }

    void End()
    {
        LogImpl(CuLog::LogLevel::None, "Exit.");
		LogThread.join();
    }
}

#define LogErr(...) LogImpl(CuLog::LogLevel::Error, __VA_ARGS__)
#define LogWarn(...) LogImpl(CuLog::LogLevel::Warn, __VA_ARGS__)
#define LogInfo(...) LogImpl(CuLog::LogLevel::Info, __VA_ARGS__)
#define LogVerb(...) LogImpl(CuLog::LogLevel::Verb, __VA_ARGS__)
#define LogDebug(...) LogImpl(CuLog::LogLevel::Debug, __VA_ARGS__)
