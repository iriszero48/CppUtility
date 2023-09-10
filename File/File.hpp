#pragma once

#include <string>
#include <filesystem>
#include <fstream>

namespace CuFile
{
    // alloc by new
    inline void ReadAllBytesAsPtr(const std::filesystem::path &path, uint8_t **ptr, size_t &size)
    {
        std::ifstream fs(path, std::ios::in | std::ios::binary);
        if (!fs)
            throw std::runtime_error("[File::ReadAllBytesAsPtr] open file failed");

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
        std::ifstream fs(path, std::ios::in | std::ios::binary);
        if (!fs)
            throw std::runtime_error("[File::ReadAllBytes] open file failed");

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
        std::ifstream fs(path, std::ios::in);
        if (!fs)
            throw std::runtime_error("[File::ReadAllText] open file failed");

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
        std::ifstream fs(path, std::ios::in);
        if (!fs)
            throw std::runtime_error("[File::ReadAllLines] open file fail");

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

    inline void WriteAllBytes(const std::filesystem::path &path, const uint8_t *data, const size_t size)
    {
        std::ofstream fs(path, std::ios::out | std::ios::binary);
        if (!fs)
            throw std::runtime_error("[File::WriteAllBytes] open file failed");

        fs.write(reinterpret_cast<const char *>(data), size);

        fs.close();
    }

    inline void WriteAllText(const std::filesystem::path &path, const std::string_view &text)
    {
        std::ofstream fs(path, std::ios::out);
        if (!fs)
            throw std::runtime_error("[File::WriteAllText] open file failed");

        fs << text;

        fs.close();
    }

    template <typename It>
    void WriteAllLines(const std::filesystem::path &path, It begin, It end)
    {
        std::ofstream fs(path, std::ios::out);
        if (!fs)
            throw std::runtime_error("[File::WriteAllLines] open file failed");

        for (auto it = begin; it != end; ++it)
        {
            fs << *it << std::endl;
        }

        fs.close();
    }
}