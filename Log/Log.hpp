#pragma once

#include "LogLevel.hpp"

#include "../Thread/Thread.hpp"

namespace CuLog
{
	template <typename T>
	class Logger
	{
	public:
		struct MsgType
		{
			LogLevel Level;
			T Msg;
		};

		LogLevel Level = LogLevel::Debug;
		CuThread::Channel<MsgType> Chan{};

		template <LogLevel Lv, typename... Args>
		void Write(Args &&...args)
		{
			if (Lv <= Level)
			{
				WriteImpl<Lv>(T{std::forward<Args>(args)...});
			}
		}

	private:
		template <LogLevel Lv>
		void WriteImpl(T &&msg)
		{
			Chan.Write(MsgType{Lv, msg});
		}
	};
};
