#pragma once

#include <algorithm>

#ifdef __cpp_lib_ranges
#define CU_RANGES_HAS_STD_RANGES
#include <ranges>
#elif defined(CU_RANGES_USE_RANGE_V3)
#define CU_RANGES_HAS_RANGE_V3
#endif

namespace CuRanges
{
	template <typename T>
	void StableSort(T& cont)
	{
#ifdef CU_RANGES_HAS_STD_RANGES
		std::ranges::stable_sort(cont);
#elif defined(CU_RANGES_HAS_RANGE_V3)

#else
		std::stable_sort(cont.begin(), cont.end());
#endif
	}

	template <typename T, typename F>
	void Generate(T& cont, F func)
	{
#ifdef CU_RANGES_HAS_STD_RANGES
		std::ranges::generate(cont, std::forward<F>(func));
#elif defined(CU_RANGES_HAS_RANGE_V3)

#else
		std::generate(cont.begin(), cont.end(), std::forward<F>(func));
#endif
	}

	template <typename T, typename F>
	decltype(auto) FindIf(const T& cont, F func)
	{
#ifdef CU_RANGES_HAS_STD_RANGES
		return std::ranges::find_if(cont, std::forward<F>(func));
#elif defined(CU_RANGES_HAS_RANGE_V3)

#else
		return std::find_if(cont.begin(), cont.end(), std::forward<F>(func));
#endif
	}
}