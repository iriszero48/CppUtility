#pragma once

#include <string>
#include <filesystem>
#include <fstream>
#include <span>

#include "../Exception/Except.hpp"

namespace CuFile
{
    CuExcept_MakeException(Exception, CuExcept, Exception);

    using FlagType = decltype(std::ios::in | std::ios::out);

    inline std::ofstream OpenForWrite(const std::filesystem::path& path, const FlagType flags = std::ios::out)
    {
        std::ofstream fs(path, flags);
        if (!fs) throw Exception("open file failed");
        return fs;
    }

    inline std::ifstream OpenForRead(const std::filesystem::path& path, const FlagType flags = std::ios::in)
    {
        std::ifstream fs(path, flags);
        if (!fs) throw Exception("open file failed");
        return fs;
    }

    inline std::fstream Open(const std::filesystem::path& path, const FlagType flags = std::ios::in | std::ios::out)
    {
        std::fstream fs(path, flags);
        if (!fs) throw Exception("open file failed");
        return fs;
    }

    // alloc by new
    inline void ReadAllBytesAsPtr(const std::filesystem::path &path, uint8_t **ptr, size_t &size)
    {
	    auto fs = OpenForRead(path, std::ios::in | std::ios::binary);

	    fs.seekg(0, std::ios::end);
        size = fs.tellg();
        fs.seekg(0, std::ios::beg);

        *ptr = new uint8_t[size];
        try
        {
            fs.read(reinterpret_cast<char *>(*ptr), size);

            fs.close();
        }
        catch (const std::exception_ptr &ex)
        {
            delete[] *ptr;
            std::rethrow_exception(ex);
        }
    }

    inline void ReadAllBytes(std::vector<uint8_t> &data, const std::filesystem::path &path)
    {
        auto fs = OpenForRead(path, std::ios::in | std::ios::binary);

        fs.seekg(0, std::ios::end);
        const auto size = fs.tellg();
        fs.seekg(0, std::ios::beg);

        const auto beforeSize = data.size();
        data.resize(beforeSize + size);
        fs.read(reinterpret_cast<char *>(data.data()) + beforeSize, size);

        fs.close();
    }

    inline std::vector<uint8_t> ReadAllBytes(const std::filesystem::path &path)
    {
        std::vector<uint8_t> ret{};
        ReadAllBytes(ret, path);
        return ret;
    }

    inline void ReadAllText(std::string &text, const std::filesystem::path &path)
    {
        auto fs = OpenForRead(path, std::ios::in);

        fs.seekg(0, std::ios::end);
        const auto size = fs.tellg();
        fs.seekg(0, std::ios::beg);

        const auto beforeSize = text.length();
        text.resize(beforeSize + size);
        fs.read(text.data() + beforeSize, size);

        fs.close();
    }

    inline std::string ReadAllText(const std::filesystem::path &path)
    {
        std::string ret{};
        ReadAllText(ret, path);
        return ret;
    }

    inline void ReadAllLines(std::vector<std::string> &lines, const std::filesystem::path &path)
    {
        auto fs = OpenForRead(path, std::ios::in);

        std::string line{};
        while (std::getline(fs, line))
            lines.push_back(line);

        fs.close();
    }

    inline std::vector<std::string> ReadAllLines(const std::filesystem::path &path)
    {
        std::vector<std::string> ret{};
        ReadAllLines(ret, path);
        return ret;
    }

    inline void WriteAllBytes(const std::filesystem::path& path, const uint8_t* data, const size_t size)
    {
        auto fs = OpenForWrite(path, std::ios::out | std::ios::binary);

    	fs.write(reinterpret_cast<const char*>(data), static_cast<std::streamsize>(size));

        fs.close();
    }

    template <size_t S>
    inline void WriteAllBytes(const std::filesystem::path& path, const std::span<uint8_t, S>& data)
    {
        WriteAllBytes(path, data.data(), data.size_bytes());
    }

    inline void WriteAllText(const std::filesystem::path &path, const std::string_view &text)
    {
        auto fs = OpenForWrite(path, std::ios::out);

        fs << text;

        fs.close();
    }

    template <typename It>
    void WriteAllLines(const std::filesystem::path &path, It begin, It end)
    {
        auto fs = OpenForWrite(path, std::ios::out);

        for (auto it = begin; it != end; ++it)
        {
            fs << *it << std::endl;
        }

        fs.close();
    }
}
