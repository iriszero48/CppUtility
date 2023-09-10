#pragma once

namespace CuCrypto
{
	template <typename T, std::size_t InputBlockSize_, std::size_t OutputBlockSize_, bool CanReuseTransform_, bool CanTransformMultipleBlocks_>
	class ITransform
	{
		bool transformed = false;

	public:
		static constexpr std::size_t InputBlockSize = InputBlockSize_;
		static constexpr std::size_t OutputBlockSize = OutputBlockSize_;
		static constexpr bool CanReuseTransform = CanReuseTransform_;
		static constexpr bool CanTransformMultipleBlocks = CanTransformMultipleBlocks_;

		std::size_t TransformCore(const std::uint8_t *input, const std::size_t size, std::uint8_t *output)
		{
			return static_cast<T *>(this)->TransformCore(input, size, output);
		}

		std::size_t Transform(const std::uint8_t *input, const std::size_t size, std::uint8_t *output)
		{
			if (!CanReuseTransform && transformed)
				throw CuCrypto_MakeExcept("!CanReuseTransform && transformed");

			if (size % InputBlockSize != 0)
				throw CuCrypto_MakeExcept("size % InputBlockSize != 0");

			if (!CanTransformMultipleBlocks && size > InputBlockSize)
				throw CuCrypto_MakeExcept("!CanTransformMultipleBlocks && size > InputBlockSize");

			const auto ret = TransformCore(input, size, output);
			transformed = true;
			return ret;
		}

		std::optional<std::size_t> GetOutputSizeCore(const std::size_t size)
		{
			return size;
		}

		std::optional<std::size_t> GetOutputSize(const std::size_t size)
		{
			if (size % InputBlockSize != 0)
				return {};

			if (!CanTransformMultipleBlocks && size > InputBlockSize)
				return {};

			return GetOutputSizeCore(size);
		}

		std::string Transform(const std::string &input)
		{
			std::string buf(GetOutputSize(input.length()).value(), 0);
			TransformCore(reinterpret_cast<const std::uint8_t *>(input.data()), input.length(), reinterpret_cast<std::uint8_t *>(buf.data()));
			return buf;
		}
	};
}
