#pragma once

#include "IStream.hpp"
#include "Exception.hpp"

#include <sstream>
#include <random>

namespace CuCrypto
{
    template <typename T>
    class EncryptStream : public IStream<EncryptStream<T>>
    {
        std::basic_stringstream<uint8_t> buffer{};
        std::basic_stringstream<uint8_t> ciphertext{};
        T transform;

    public:
        EncryptStream(T transform) : transform(std::move(transform)) {}

        void WriteCore(const std::uint8_t *data, const std::size_t size)
        {
            if (this->Finished)
                throw CuCrypto_MakeExcept("finished");

            buffer.write(data, size);

            if (this->Cipher == CipherMode::None && !transform.CanReuseTransform)
                return;

            buffer.seekg(0, std::ios::end);
            auto bufLen = buffer.tellg();
            buffer.seekg(0, std::ios::beg);

            if (this->Cipher == CipherMode::None || this->Cipher == CipherMode::ECB)
            {
                while (bufLen > T::InputBlockSize)
                {
                    uint8_t buf[T::InputBlockSize]{};
                    buffer.read(buf, T::InputBlockSize);

                    uint8_t out[T::OutputBlockSize];
                    transform.Transform(buf, T::InputBlockSize, out);

                    ciphertext.write(out, T::OutputBlockSize);

                    bufLen -= transform.InputBlockSize;
                }
            }
            else if (this->Cipher == CipherMode::CBC)
            {
                if (T::InputBlockSize != T::OutputBlockSize)
                    throw CuCrypto_MakeExcept("T::InputBlockSize != T::OutputBlockSize");
                if (this->IV.size() != T::InputBlockSize)
                    throw CuCrypto_MakeExcept("IV.size() != T::InputBlockSize");

                while (bufLen > T::InputBlockSize)
                {
                    uint8_t buf[T::InputBlockSize]{};
                    buffer.read(buf, T::InputBlockSize);

                    for (size_t i = 0; i < T::InputBlockSize; ++i)
                        buf[i] ^= this->IV[i];

                    uint8_t out[T::OutputBlockSize];
                    transform.Transform(buf, T::InputBlockSize, out);

                    ciphertext.write(out, T::OutputBlockSize);
                    this->IV.resize(T::OutputBlockSize);
                    std::copy_n(out, T::OutputBlockSize, this->IV.data());

                    bufLen -= T::InputBlockSize;
                }
            }
            else if (this->Cipher == CipherMode::CFB)
            {
                if (T::InputBlockSize != T::OutputBlockSize)
                    throw CuCrypto_MakeExcept("T::InputBlockSize != T::OutputBlockSize");
                if (this->IV.size() != T::InputBlockSize)
                    throw CuCrypto_MakeExcept("IV.size() != T::InputBlockSize");

                while (bufLen > T::OutputBlockSize)
                {
                    uint8_t out[T::OutputBlockSize];
                    transform.Transform(this->IV.data(), T::InputBlockSize, out);

                    uint8_t buf[T::OutputBlockSize]{};
                    buffer.read(buf, T::OutputBlockSize);

                    for (size_t i = 0; i < T::OutputBlockSize; ++i)
                        buf[i] ^= out[i];

                    ciphertext.write(buf, T::OutputBlockSize);
                    this->IV.assign(buf, buf + T::OutputBlockSize);

                    bufLen -= T::OutputBlockSize;
                }
            }
            else if (this->Cipher == CipherMode::OFB)
            {
                if (T::InputBlockSize != T::OutputBlockSize)
                    throw __Cryptography_Ex__(Exception, "T::InputBlockSize != T::OutputBlockSize");
                if (IV.size() != T::InputBlockSize)
                    throw __Cryptography_Ex__(Exception, "IV.size() != T::InputBlockSize");

                while (bufLen > T::OutputBlockSize)
                {
                    uint8_t out[T::OutputBlockSize];
                    transform.Transform(this->IV.data(), T::InputBlockSize, out);
                    this->IV.assign(out, out + T::OutputBlockSize);

                    uint8_t buf[T::OutputBlockSize]{};
                    buffer.read(buf, T::OutputBlockSize);

                    for (size_t i = 0; i < T::OutputBlockSize; ++i)
                        buf[i] ^= out[i];

                    ciphertext.write(buf, T::OutputBlockSize);

                    bufLen -= T::OutputBlockSize;
                }
            }
        }

        size_t ReadCore(std::uint8_t *data, const std::size_t size)
        {
            if (this->Cipher == CipherMode::None && !transform.CanReuseTransform)
            {
                const auto raw = buffer.str();
                if (!raw.empty())
                {
                    const auto pad = PaddingData(raw.data(), raw.length(), this->Padding, transform.InputBlockSize);
                    const auto len = transform.GetOutputSize(pad.size()).value();
                    const auto outBuf = std::make_unique<uint8_t>(len);
                    transform.Transform(pad.data(), pad.size(), outBuf.get());
                    ciphertext.write(outBuf.get(), len);
                }
            }

            ciphertext.read(data, size);
            return ciphertext.gcount();
        }

    private:
        static std::vector<std::uint8_t> PaddingData(const std::uint8_t *input, const std::size_t size, const PaddingMode padding, const std::size_t chunkSize)
        {
            std::vector buf(input, input + size);
            const auto rem = size % chunkSize;

            if (rem || padding == PaddingMode::None)
                return buf;

            const auto pos = input + size - rem;
            const auto padSize = chunkSize - rem;
            switch (padding)
            {
            case PaddingMode::ANSIX923:
                for (std::size_t i = 0; i < padSize - 1; i++)
                    buf.push_back(0);

                buf.push_back(padSize);

                return buf;

            case PaddingMode::ISO10126:
                std::random_device rnd{};

                for (std::size_t i = 0; i < padSize - 1; i++)
                    buf.push_back(rnd() % 256);

                buf.push_back(padSize);

                return buf;

            case PaddingMode::PKCS7:
                for (std::size_t i = 0; i < padSize; i++)
                    buf.push_back(padSize);

                return buf;

            case PaddingMode::Zeros:
                for (std::size_t i = 0; i < padSize; i++)
                    buf.push_back(0);

                return buf;

            default:
                throw CuCrypto_MakeExcept("unknow PaddingMode");
            }
        }

        void Transform(uint8_t data[T::InputBlockSize])
        {
            if (this->Cipher == CipherMode::None || this->Cipher == CipherMode::ECB)
            {
            }
        }
    };
}