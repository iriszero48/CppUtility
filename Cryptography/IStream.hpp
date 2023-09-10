#pragma once

#include <vector>

namespace CuCrypto
{
    enum class CipherMode
    {
        None,
        ECB,
        CBC,
        CFB,
        OFB
    };

    enum class PaddingMode
    {
        None,
        PKCS7,
        Zeros,
        ANSIX923,
        ISO10126
    };

    template <typename T>
    class IStream
    {
    public:
        CipherMode Cipher = CipherMode::None;
        PaddingMode Padding = PaddingMode::None;
        std::vector<std::uint8_t> IV;
        bool Finished = false;

        T &SetIV(const std::uint8_t *iv, const std::size_t size)
        {
            IV.resize(size);
            std::copy_n(iv, size, IV.data());
            return *static_cast<T *>(this);
        }

        T &SetCipher(const CipherMode cipher)
        {
            Cipher = cipher;
            return *static_cast<T *>(this);
        }

        T &SetPadding(const PaddingMode padding)
        {
            Padding = padding;
            return *static_cast<T *>(this);
        }

        T &Finish()
        {
            Finished = true;
            return *static_cast<T *>(this);
        }

        void WriteCore(const std::uint8_t *data, const std::size_t size)
        {
            static_cast<T *>(this)->WriteCore(data, size);
        }

        T &Write(const std::uint8_t *data, const std::size_t size)
        {
            WriteCore(data, size);
            return *static_cast<T *>(this);
        }

        size_t ReadCore(const std::uint8_t *data, const std::size_t size)
        {
            return static_cast<T *>(this)->ReadCore(data, size);
        }

        size_t Read(uint8_t *data, const size_t size)
        {
            return ReadCore(data, size);
        }

        size_t GetBufferCiphertextLength() const
        {
            return static_cast<T *>(this)->GetBufferCiphertextLength();
        }

        std::vector<uint8_t> Read()
        {
            std::vector<uint8_t> buf(GetBufferCiphertextLength());
            Read(buf.data(), buf.size());
            return buf;
        }
    };
}
