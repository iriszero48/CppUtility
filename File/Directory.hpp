#pragma once

namespace CuDirectory
{
	enum IteratorOptions
	{
		IteratorOptions_None = 0,

		IteratorOptions_RecurseSubdirectories = 1 << 0,
	};

	namespace Detail
	{
		template <bool Rec>
		auto GetIterator(const std::filesystem::path &path)
		{
			if constexpr (Rec)
				return std::filesystem::recursive_directory_iterator(path);
			else
				return std::filesystem::directory_iterator(path);
		}

		template <bool Rec, typename Func>
		void WalkImpl(const std::filesystem::path &path, IteratorOptions options, Func func)
		{
			for (const auto &it : GetIterator<Rec>(path))
				func(it);
		}

		template <typename Func>
		void Walk(const std::filesystem::path &path, IteratorOptions options, Func func)
		{
			if (options & IteratorOptions_RecurseSubdirectories)
				WalkImpl<true>(path, options, func);
			else
				WalkImpl<false>(path, options, func);
		}
	}

	inline std::vector<std::filesystem::path> GetFiles(const std::filesystem::path &path, IteratorOptions options = IteratorOptions_None)
	{
		std::vector<std::filesystem::path> files;
		Detail::Walk(path, options, [&](const auto &it)
					 {
				if (it.is_regular_file()) files.emplace_back(it.path()); });
		return files;
	}
}