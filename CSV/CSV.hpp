#pragma once

#include <fstream>

#include "Exception/Except.hpp"
#include "File/File.hpp"

namespace CuCSV
{
	CuExcept_MakeException(Exception, CuExcept, Exception);

	struct Setting
	{
		std::string Separator = ",";
		std::string LineSeparator = "\n";
		char Delim = '"';
		char Escape = '\\';
	};

	namespace Detail
	{
		template <typename T>
		inline constexpr auto IsStringType = CuUtil::IsAnyOfV<T, std::string, std::string_view, const char*, char*>;
	}

	class Writer
	{
		std::ofstream fs;
		bool isFirstOfRow = true;

	public:
		Setting WriterSetting{};

		Writer() = default;

		Writer(const std::filesystem::path& path, bool append = false, bool startNextLine = false)
		{
			Open(path, append, startNextLine);
		}

		Writer(const Writer& file) = delete;
		Writer(Writer&&) = default;

		Writer& operator=(const Writer& file) = delete;
		Writer& operator=(Writer&&) = default;

		~Writer() = default;

		void Open(const std::filesystem::path& path, bool append = false, bool startNewRow = false)
		{
			auto flag = std::ios::out;
			if (append) flag |= std::ios::app;

			fs = CuFile::OpenForWrite(path, flag);

			if (startNewRow) EndRow();
		}

		void Flush()
		{
			fs.flush();
		}

		void Close()
		{
			fs.close();
		}

		void EndRow()
		{
			fs << WriterSetting.LineSeparator;
			isFirstOfRow = true;
		}

		Writer& operator << (Writer& (*val)(Writer&))
		{
			return val(*this);
		}

		template<typename T>
		Writer& operator << (const T& col)
		{
			return Write(col);
		}

		Writer& EndRow(Writer& writer)
		{
			writer.EndRow();
			return writer;
		}

		template <typename T>
		Writer& Write(const T& col)
		{
			if constexpr (Detail::IsStringType<T>)
			{
				return WriteImpl(Quoted(col));
			}

			return WriteImpl(col);
		}

		template <typename... Args>
		Writer& WriteRow(Args&&... col)
		{
			(Write(std::forward<Args>(col)),...);
			EndRow();
			return *this;
		}

		template<typename T>
		[[nodiscard]] std::string Quoted(const T& val) const
		{
			return CuStr::Combine(std::quoted(val, WriterSetting.Delim, WriterSetting.Escape));
		}

	private:
		template<typename T>
		Writer& WriteImpl(const T& val)
		{
			if (!isFirstOfRow)
			{
				fs << WriterSetting.Separator;
			}
			else
			{
				isFirstOfRow = false;
			}
			fs << val;
			return *this;
		}
	};
}