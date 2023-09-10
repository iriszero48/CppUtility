#pragma once

#include "../Utility/Utility.hpp"

#include <algorithm>
#include <numeric>

// ParallellUseTbb
// ParallelUseOpenMP
// ParallelUseStb
// ParallelUseSingleThread

#ifdef ParallelUseTbb
#define __ParallelUseTbb
#elif defined(ParallelUseOpenMP)
#define __ParallelUseOpenMP
#elif defined(ParallelUseStb)
#define __ParallelUseStb
#elif defined(ParallelUseSingleThread)
#define __ParallelUseSingleThread
#endif

#ifdef __ParallelUseTbb
#include <optional>

#include <tbb/parallel_for.h>
#include <tbb/parallel_reduce.h>
#include <tbb/parallel_sort.h>
#include <tbb/spin_mutex.h>

#elif defined(__ParallelUseOpenMP)
#include <omp.h>
#elif defined(__ParallelUseStb)
#include <future>
#include <optional>
#include <vector>
#elif defined(__ParallelUseSingleThread)
#else
#include <execution>
#endif

namespace Parallel
{
    namespace _Detail
    {
#ifdef __ParallelUseTbb
        template <typename Iter, typename Func>
        void ForEachTbb(Iter beg, Iter end, Func func)
        {
            tbb::parallel_for(tbb::blocked_range(beg, end),
                              [=](tbb::blocked_range<Iter> &rng)
                              {
                                  std::for_each(rng.begin(), rng.end(), func);
                              });
        }
#endif

#ifdef __ParallelUseOpenMP
        template <typename Iter, typename Func>
        void ForEachOpenMP(Iter beg, Iter end, Func func)
        {
            const auto size = end - beg;
#pragma omp parallel for
            for (std::size_t i = 0; i < size; ++i)
            {
                func(*(beg + i));
            }
        }
#endif

#ifdef __ParallelUseStb
        template <typename Iter, typename Func>
        void ForEachStb(Iter beg, Iter end, Func func)
        {
            const auto size = end - beg;
            const auto threadCount = std::min<std::size_t>(std::thread::hardware_concurrency(), size);
            const auto chunkSize = size / threadCount;
            std::vector<std::future<void>> futures;
            futures.reserve(threadCount);
            for (std::size_t i = 0; i < threadCount; ++i)
            {
                futures.emplace_back(std::async(std::launch::async,
                                                [=]()
                                                {
                                                    const auto sb = beg + i * chunkSize;
                                                    const auto se = beg + ((i == threadCount - 1) ? size : (i + 1) * chunkSize);
                                                    std::for_each(sb, se, func);
                                                }));
            }

            for (auto &future : futures)
                future.get();
        }
#endif
    }

    template <typename Iter, typename Func>
    void ForEach(Iter beg, Iter end, Func func)
    {
#ifdef __ParallelUseTbb
        _Detail::ForEachTbb(beg, end, func);
#elif defined(__ParallelUseOpenMP)
        _Detail::ForEachOpenMP(beg, end, func);
#elif defined(__ParallelUseStb)
        _Detail::ForEachStb(beg, end, func);
#elif defined(__ParallelUseSingleThread)
        std::for_each(beg, end, func);
#else
        std::for_each(std::execution::par_unseq, beg, end, func);
#endif
    }

    namespace _Detail
    {
#ifdef __ParallelUseTbb
        template <typename Iter1, typename Iter2, typename Func>
        void MapTbb(Iter1 beg, Iter1 end, Iter2 dst, Func func)
        {
            tbb::parallel_for(tbb::blocked_range(beg, end),
                              [=](tbb::blocked_range<Iter1> &rng)
                              {
                                  const auto b = rng.begin();
                                  const auto e = rng.end();
                                  const auto offset = b - beg;
                                  std::transform(b, e, dst + offset, func);
                              });
        }
#endif

#ifdef __ParallelUseOpenMP
        template <typename Iter1, typename Iter2, typename Func>
        void MapOpenMP(Iter1 beg, Iter1 end, Iter2 dst, Func func)
        {
            const auto size = end - beg;
#pragma omp parallel for
            for (std::size_t i = 0; i < size; ++i)
            {
                *(dst + i) = func(*(beg + i));
            }
        }
#endif

#ifdef __ParallelUseStb
        template <typename Iter1, typename Iter2, typename Func>
        void MapStb(Iter1 beg, Iter1 end, Iter2 dst, Func func)
        {
            const auto size = end - beg;
            if (size == 0)
                return;

            const auto threadCount = std::min<std::size_t>(std::thread::hardware_concurrency(), size);
            const auto chunkSize = size / threadCount;

            std::vector<std::future<void>> futures{};
            for (std::size_t i = 0; i < threadCount; ++i)
            {
                futures.emplace_back(std::async(std::launch::async,
                                                [=]()
                                                {
                                                    const auto sb = beg + i * chunkSize;
                                                    const auto se = beg + ((i == threadCount - 1) ? size : (i + 1) * chunkSize);
                                                    const auto offset = sb - beg;
                                                    std::transform(sb, se, dst + offset, func);
                                                }));
            }

            for (auto &f : futures)
                f.get();
        }
#endif
    }

    template <typename Iter1, typename Iter2, typename Func>
    void Map(Iter1 beg, Iter1 end, Iter2 dst, Func func)
    {
#ifdef __ParallelUseTbb
        _Detail::MapTbb(beg, end, dst, func);
#elif defined(__ParallelUseOpenMP)
        _Detail::MapOpenMP(beg, end, dst, func);
#elif defined(__ParallelUseStb)
        _Detail::MapStb(beg, end, dst, func);
#elif defined(__ParallelUseSingleThread)
        std::transform(beg, end, dst, func);
#else
        std::transform(std::execution::par_unseq, beg, end, dst, func);
#endif
    }

    namespace _Detail
    {
#ifdef __ParallelUseTbb
        template <typename Iter1, typename Iter2>
        void CopyTbb(Iter1 beg, Iter1 end, Iter2 dst)
        {
            tbb::parallel_for(tbb::blocked_range(beg, end),
                              [=](tbb::blocked_range<Iter1> &rng)
                              {
                                  const auto b = rng.begin();
                                  const auto e = rng.end();
                                  const auto offset = b - beg;
                                  std::copy(b, e, dst + offset);
                              });
        }
#endif

#ifdef __ParallelUseOpenMP
        template <typename Iter1, typename Iter2>
        void CopyOpenMP(Iter1 beg, Iter1 end, Iter2 dst)
        {
            using T = typename std::iterator_traits<Iter2>::value_type;

            const auto size = end - beg;

#pragma omp parallel for
            for (std::size_t i = 0; i < size; ++i)
            {
                T val = *(beg + i);
                *(dst + i) = std::move(val);
            }
        }
#endif

#ifdef __ParallelUseStb
        template <typename Iter1, typename Iter2>
        void CopyStb(Iter1 beg, Iter1 end, Iter2 dst)
        {
            const auto size = end - beg;
            const auto threadCount = std::min<std::size_t>(std::thread::hardware_concurrency(), size);
            const auto chunkSize = size / threadCount;
            std::vector<std::future<void>> futures;
            futures.reserve(threadCount);
            for (std::size_t i = 0; i < threadCount; ++i)
            {
                futures.emplace_back(std::async(std::launch::async,
                                                [=]()
                                                {
                                                    const auto sb = beg + i * chunkSize;
                                                    const auto se = beg + ((i == threadCount - 1) ? size : (i + 1) * chunkSize);
                                                    const auto offset = sb - beg;
                                                    std::copy(sb, se, dst + offset);
                                                }));
            }
            for (auto &f : futures)
                f.get();
        }
#endif
    }

    template <typename Iter1, typename Iter2>
    void Copy(Iter1 beg, Iter1 end, Iter2 dst)
    {
#ifdef __ParallelUseTbb
        _Detail::CopyTbb(beg, end, dst);
#elif defined(__ParallelUseOpenMP)
        _Detail::CopyOpenMP(beg, end, dst);
#elif defined(__ParallelUseStb)
        _Detail::CopyStb(beg, end, dst);
#elif defined(__ParallelUseSingleThread)
        std::copy(beg, end, dst);
#else
        std::copy(std::execution::par_unseq, beg, end, dst);
#endif
    }

    namespace _Detail
    {
#ifdef __ParallelUseTbb
        template <typename Iter1, typename Iter2>
        void CopyNTbb(Iter1 beg, const std::size_t size, Iter2 dst)
        {
            tbb::parallel_for(tbb::blocked_range(beg, beg + size),
                              [=](tbb::blocked_range<Iter1> &rng)
                              {
                                  const auto b = rng.begin();
                                  const auto e = rng.end();
                                  const auto offset = b - beg;
                                  std::copy(b, e, dst + offset);
                              });
        }
#endif

#ifdef __ParallelUseOpenMP
        template <typename Iter1, typename Iter2>
        void CopyNOpenMP(Iter1 beg, const std::size_t size, Iter2 dst)
        {
            using T = typename std::iterator_traits<Iter2>::value_type;

#pragma omp parallel for
            for (std::size_t i = 0; i < size; ++i)
            {
                T val = *(beg + i);
                *(dst + i) = std::move(val);
            }
        }
#endif

#ifdef __ParallelUseStb
        template <typename Iter1, typename Iter2>
        void CopyNStb(Iter1 beg, const std::size_t size, Iter2 dst)
        {
            _Detail::CopyStb(beg, beg + size, dst);
        }
#endif
    }

    template <typename Iter1, typename Iter2>
    void CopyN(Iter1 beg, const std::size_t size, Iter2 dst)
    {
#ifdef __ParallelUseTbb
        _Detail::CopyNTbb(beg, size, dst);
#elif defined(__ParallelUseOpenMP)
        _Detail::CopyNOpenMP(beg, size, dst);
#elif defined(__ParallelUseStb)
        _Detail::CopyNStb(beg, size, dst);
#elif defined(__ParallelUseSingleThread)
        std::copy_n(beg, size, dst);
#else
        std::copy_n(std::execution::par_unseq, beg, size, dst);
#endif
    }

    namespace _Detail
    {
#ifdef __ParallelUseTbb
        template <typename Iter1, typename Iter2, typename Func>
        Iter2 CopyIfTbb(Iter1 beg, Iter1 end, Iter2 dst, Func func)
        {
            using T = typename std::iterator_traits<Iter2>::value_type;

            std::vector<std::optional<T>> masks(end - beg);
            const auto masksBeg = masks.begin();
            tbb::parallel_for(tbb::blocked_range(beg, end),
                              [=](tbb::blocked_range<Iter1> &rng)
                              {
                                  const auto b = rng.begin();
                                  const auto e = rng.end();
                                  const auto offset = b - beg;
                                  const auto size = e - b;

                                  for (std::size_t i = 0; i < size; ++i)
                                  {
                                      if (func(*(b + i)))
                                      {
                                          T res = *(b + i);
                                          *(masksBeg + offset + i) = std::move(res);
                                      }
                                  }
                              });

            std::for_each(masks.begin(), masks.end(),
                          [&](auto &v)
                          {
                              if (v.has_value())
                              {
                                  *dst = std::move(*v);
                                  ++dst;
                              }
                          });

            return dst;
        }
#endif

#ifdef __ParallelUseOpenMP
        template <typename Iter1, typename Iter2, typename Func>
        Iter2 CopyIfOpenMP(Iter1 beg, Iter1 end, Iter2 dst, Func func)
        {
            using T = typename std::iterator_traits<Iter2>::value_type;

            const auto size = end - beg;

#pragma omp parallel for
            for (std::size_t i = 0; i < size; ++i)
            {
                if (func(*(beg + i)))
                {
                    T val = *(beg + i);

#pragma omp critical
                    {
                        *dst = std::move(val);
                        ++dst;
                    }
                }
            }

            return dst;
        }
#endif

#ifdef __ParallelUseStb
        template <typename Iter1, typename Iter2, typename Func>
        Iter2 CopyIfStb(Iter1 beg, Iter1 end, Iter2 dst, Func func)
        {
            using T = typename std::iterator_traits<Iter2>::value_type;

            std::vector<std::optional<T>> masks(end - beg);
            _Detail::MapStb(beg, end, masks.begin(),
                            [&](const auto &v) -> std::optional<T>
                            {
                                if (func(v))
                                {
                                    T res = v;
                                    return std::move(res);
                                }
                                else
                                {
                                    return std::nullopt;
                                }
                            });

            std::for_each(masks.begin(), masks.end(),
                          [&](auto &v)
                          {
                              if (v.has_value())
                              {
                                  *dst = std::move(*v);
                                  ++dst;
                              }
                          });

            return dst;
        }
#endif
    }

    template <typename Iter1, typename Iter2, typename Func>
    Iter2 CopyIf(Iter1 beg, Iter1 end, Iter2 dst, Func func)
    {
#ifdef __ParallelUseTbb
        return _Detail::CopyIfTbb(beg, end, dst, func);
#elif defined(__ParallelUseOpenMP)
        return _Detail::CopyIfOpenMP(beg, end, dst, func);
#elif defined(__ParallelUseStb)
        return _Detail::CopyIfStb(beg, end, dst, func);
#elif defined(__ParallelUseSingleThread)
        return std::copy_if(beg, end, dst, func);
#else
        return std::copy_if(std::execution::par_unseq, beg, end, dst, func);
#endif
    }

    namespace _Detail
    {
#ifdef __ParallelUseTbb
        template <typename Iter, typename Func>
        struct MaxElementBody
        {
            Iter res{};

            Func func;

            MaxElementBody(Func func) : func(func) {}
            MaxElementBody(MaxElementBody &body, tbb::split) : func(body.func) {}

            void operator()(tbb::blocked_range<Iter> &rng)
            {
                res = std::max_element(rng.begin(), rng.end(), func);
            }

            void join(const MaxElementBody &val)
            {
                if (func(*res, *val.res))
                    res = val.res;
            }
        };

        template <typename Iter, typename Func>
        Iter MaxElementTbb(Iter beg, Iter end, Func func)
        {
            _Detail::MaxElementBody<Iter, Func> body(func);
            tbb::parallel_reduce(tbb::blocked_range(beg, end), body);
            return body.res;
        }
#endif

#ifdef __ParallelUseOpenMP
        template <typename Iter, typename Func>
        Iter MaxElementOpenMP(Iter beg, Iter end, Func func)
        {
            const auto size = end - beg;

            const auto thxNum = omp_get_max_threads();
            auto maxValues = std::vector<Iter>(thxNum, beg);

#pragma omp parallel shared(maxValues)
            {
                const auto id = omp_get_thread_num();

#pragma omp for
                for (std::size_t i = 0; i < size; ++i)
                {
                    if (auto &maxVal = maxValues[id]; !func(*(beg + i), *maxVal))
                        maxVal = beg + i;
                }
            }

            return *std::max_element(maxValues.begin(), maxValues.end(),
                                     [&](auto &l, auto &r)
                                     { return func(*l, *r); });
        }
#endif

#ifdef __ParallelUseStb
        template <typename Iter, typename Func>
        Iter MaxElementStb(Iter beg, Iter end, Func func)
        {
            const auto size = end - beg;
            const auto threadCount = std::min<std::size_t>(std::thread::hardware_concurrency(), size);
            const auto chunkSize = size / threadCount;
            std::vector<std::future<Iter>> futures;
            futures.reserve(threadCount);
            for (std::size_t i = 0; i < threadCount; ++i)
            {
                futures.emplace_back(std::async(std::launch::async,
                                                [=]()
                                                {
                                                    const auto sb = beg + i * chunkSize;
                                                    const auto se = beg + ((i == threadCount - 1) ? size : (i + 1) * chunkSize);
                                                    return std::max_element(sb, se, func);
                                                }));
            }
            std::vector<Iter> maxValues{};
            for (auto &f : futures)
                maxValues.emplace_back(f.get());

            return *std::max_element(maxValues.begin(), maxValues.end(),
                                     [&](auto &l, auto &r)
                                     { return func(*l, *r); });
        }
#endif
    } // namespace _Detail

    template <typename Iter, typename Func>
    Iter MaxElement(Iter beg, Iter end, Func func)
    {
#ifdef __ParallelUseTbb
        return _Detail::MaxElementTbb(beg, end, func);
#elif defined(__ParallelUseOpenMP)
        return _Detail::MaxElementOpenMP(beg, end, func);
#elif defined(__ParallelUseStb)
        return _Detail::MaxElementStb(beg, end, func);
#elif defined(__ParallelUseSingleThread)
        return std::max_element(beg, end, func);
#else
        return std::max_element(std::execution::par_unseq, beg, end, func);
#endif
    }

    namespace _Detail
    {
#ifdef __ParallelUseTbb
        template <typename Iter, typename... Args>
        void SortTbb(Iter beg, Iter end, Args &&...args)
        {
            tbb::parallel_sort(beg, end, std::forward<Args>(args)...);
        }
#endif

#ifdef __ParallelUseOpenMP
        template <typename Iter, typename... Args>
        void SortOpenMP(Iter beg, Iter end, Args &&...args)
        {
            const auto size = end - beg;

            const auto thxNum = omp_get_max_threads();
            const auto chunkSize = size / thxNum;
            std::vector<Iter> begVec{};
            for (std::size_t i = 0; i < thxNum; ++i)
                begVec.emplace_back(beg + i * chunkSize);

            begVec.emplace_back(end);

#pragma omp parallel for
#pragma omp parallel shared(begVec)
            for (std::size_t i = 0; i < thxNum; ++i)
            {
                const auto se = beg + ((i == thxNum - 1) ? size : (i + 1) * chunkSize);
                std::sort(begVec[i], se, std::forward<Args>(args)...);
            }

            while (begVec.size() > 2)
            {
#pragma omp parallel for
                for (std::size_t i = 0; i < begVec.size() - 2; i += 2)
                {
                    std::inplace_merge(begVec[i], begVec[i + 1], begVec[i + 2], std::forward<Args>(args)...);
                }

                std::vector<Iter> newBegVec{};
                for (std::size_t i = 0; i < begVec.size(); i += 2)
                    newBegVec.emplace_back(begVec[i]);

                begVec = newBegVec;
            }
        }
#endif

#ifdef __ParallelUseStb
        template <typename Iter, typename... Args>
        void SortStbImpl(const std::vector<std::future<void>> &vec, std::size_t i, Args... args)
        {
            std::sort(vec[i], vec[i + 1], std::forward<Args>(args)...);
        }

        template <typename Iter, typename Func>
        void SortStb(Iter beg, Iter end, Func func)
        {
            const auto size = end - beg;
            if (size <= 1)
                return;

            const auto threadCount = std::min<std::size_t>(std::thread::hardware_concurrency(), size);
            const auto chunkSize = size / threadCount;

            std::vector<Iter> begVec{};
            for (std::size_t i = 0; i < threadCount; ++i)
                begVec.emplace_back(beg + i * chunkSize);

            begVec.emplace_back(end);

            std::vector<std::future<void>> futures{};
            for (std::size_t i = 0; i < threadCount; ++i)
            {
                futures.push_back(std::async(std::launch::async, [=]()
                                             { std::sort(begVec[i], begVec[i + 1], func); }));
            }
            for (auto &f : futures)
                f.get();
            futures.clear();

            while (begVec.size() > 2)
            {
                for (std::size_t i = 0; i + 2 < begVec.size(); i += 2)
                {
                    futures.emplace_back(std::async(std::launch::async,
                                                    [=]()
                                                    {
                                                        std::inplace_merge(begVec[i], begVec[i + 1], begVec[i + 2], func);
                                                    }));
                }
                for (auto &f : futures)
                    f.get();

                std::vector<Iter> newBegVec{};

                for (std::size_t i = 0; i < begVec.size(); i += 2)
                    newBegVec.emplace_back(begVec[i]);

                if (begVec.size() > 3 && (begVec.size() & 1) == 0)
                    newBegVec.emplace_back(end);

                begVec = newBegVec;
                futures.clear();
            }
        }
#endif
    } // namespace _Detail

    template <typename Iter>
    void Sort(Iter beg, Iter end)
    {
#ifdef __ParallelUseTbb
        _Detail::SortTbb(beg, end);
#elif defined(__ParallelUseOpenMP)
        _Detail::SortOpenMP(beg, end);
#elif defined(__ParallelUseStb)
        _Detail::SortStb(beg, end, std::less<>{});
#elif defined(__ParallelUseSingleThread)
        std::sort(beg, end);
#else
        std::sort(std::execution::par_unseq, beg, end);
#endif
    }

    template <typename Iter, typename Func>
    void Sort(Iter beg, Iter end, Func func)
    {
#ifdef __ParallelUseTbb
        _Detail::SortTbb(beg, end, func);
#elif defined(__ParallelUseOpenMP)
        _Detail::SortOpenMP(beg, end, func);
#elif defined(__ParallelUseStb)
        _Detail::SortStb(beg, end, func);
#elif defined(__ParallelUseSingleThread)
        std::sort(beg, end, func);
#else
        std::sort(std::execution::par_unseq, beg, end, func);
#endif
    }

    template <typename Iter, typename Func>
    Iter Unique(Iter beg, Iter end, Func func)
    {
#ifdef __ParallelUseTbb
        // @todo
        return std::unique(beg, end, func);
#elif defined(__ParallelUseOpenMP)
        // @todo
        return std::unique(beg, end, func);
#elif defined(__ParallelUseStb)
        // @todo
        return std::unique(beg, end, func);
#elif defined(__ParallelUseSingleThread)
        return std::unique(beg, end, func);
#else
        return std::unique(std::execution::par_unseq, beg, end, func);
#endif
    }

    template <typename Iter>
    Iter Unique(Iter beg, Iter end)
    {
#ifdef __ParallelUseTbb
        // @todo
        return std::unique(beg, end);
#elif defined(__ParallelUseOpenMP)
        // @todo
        return std::unique(beg, end);
#elif defined(__ParallelUseStb)
        // @todo
        return std::unique(beg, end);
#elif defined(__ParallelUseSingleThread)
        return std::unique(beg, end);
#else
        return std::unique(std::execution::par_unseq, beg, end);
#endif
    }

    namespace _Detail
    {
#ifdef __ParallelUseTbb
        template <typename Iter, typename Func>
        decltype(auto) CountIfTbb(Iter beg, Iter end, Func func)
        {
            using T = typename std::iterator_traits<Iter>::difference_type;

            return tbb::parallel_reduce(
                tbb::blocked_range(beg, end), static_cast<T>(0),
                [func](const auto &rng, T acc)
                {
                    return std::count_if(rng.begin(), rng.end(), func) + acc;
                },
                std::plus<>{});
        }
#endif

#ifdef __ParallelUseOpenMP
        template <typename Iter, typename Func>
        decltype(auto) CountIfOpenMP(Iter beg, Iter end, Func func)
        {
            using T = typename std::iterator_traits<Iter>::difference_type;

            const auto size = end - beg;
            T count = 0;

#pragma omp parallel for reduction(+ : count)
            for (std::size_t i = 0; i < size; ++i)
            {
                count += func(*(beg + i)) ? 1 : 0;
            }

            return count;
        }
#endif

#ifdef __ParallelUseStb
        template <typename Iter, typename Func>
        decltype(auto) CountIfStb(Iter beg, Iter end, Func func)
        {
            using T = typename std::iterator_traits<Iter>::difference_type;

            const auto size = end - beg;
            if (size <= 0)
                return T(0);

            const auto threadCount = std::min<std::size_t>(std::thread::hardware_concurrency(), size);
            const auto chunkSize = size / threadCount;

            std::vector<std::future<T>> futures;
            for (std::size_t i = 0; i < threadCount; ++i)
            {
                futures.emplace_back(std::async(std::launch::async,
                                                [=]()
                                                {
                                                    const auto sb = beg + i * chunkSize;
                                                    const auto se = beg + ((i == threadCount - 1) ? size : (i + 1) * chunkSize);
                                                    return std::count_if(sb, se, func);
                                                }));
            }

            std::vector<T> countValues{};
            for (auto &f : futures)
                countValues.emplace_back(f.get());

            return std::accumulate(countValues.begin(), countValues.end(), static_cast<T>(0));
        }
#endif
    }

    template <typename Iter, typename Func>
    typename std::iterator_traits<Iter>::difference_type CountIf(Iter beg, Iter end, Func func)
    {
#ifdef __ParallelUseTbb
        return _Detail::CountIfTbb(beg, end, func);
#elif defined(__ParallelUseOpenMP)
        return _Detail::CountIfOpenMP(beg, end, func);
#elif defined(__ParallelUseStb)
        return _Detail::CountIfStb(beg, end, func);
#elif defined(__ParallelUseSingleThread)
        return std::count_if(beg, end, func);
#else
        return std::count_if(std::execution::par_unseq, beg, end, func);
#endif
    }

    namespace _Detail
    {
#ifdef __ParallelUseTbb
        template <typename Iter>
        struct ReduceBody
        {
            using T = typename std::iterator_traits<Iter>::value_type;

            T res{};

            ReduceBody() {}
            ReduceBody(ReduceBody &, tbb::split) {}

            void operator()(tbb::blocked_range<Iter> &rng)
            {
                res = std::reduce(rng.begin(), rng.end(), res);
            }

            void join(const ReduceBody &val) { res += val.res; }
        };

        template <typename Iter>
        decltype(auto) ReduceTbb(Iter beg, Iter end)
        {
            _Detail::ReduceBody<Iter> body{};
            tbb::parallel_reduce(tbb::blocked_range(beg, end), body);
            return body.res;
        }
#endif

#ifdef __ParallelUseOpenMP
        template <typename Iter>
        decltype(auto) ReduceOpenMP(Iter beg, Iter end)
        {
            using T = typename std::iterator_traits<Iter>::value_type;

            const auto size = end - beg;
            T res = 0;

#pragma omp parallel for reduction(+ : res)
            for (std::size_t i = 0; i < size; ++i)
            {
                res += *(beg + i);
            }

            return res;
        }
#endif

#ifdef __ParallelUseStb
        template <typename Iter>
        decltype(auto) ReduceStb(Iter beg, Iter end)
        {
            using T = typename std::iterator_traits<Iter>::value_type;

            const auto size = end - beg;
            const auto threadCount = std::min<std::size_t>(std::thread::hardware_concurrency(), size);
            const auto chunkSize = size / threadCount;

            std::vector<std::future<T>> futures;
            for (std::size_t i = 0; i < threadCount; ++i)
            {
                futures.emplace_back(std::async(std::launch::async,
                                                [=]()
                                                {
                                                    const auto sb = beg + i * chunkSize;
                                                    const auto se = beg + ((i == threadCount - 1) ? size : (i + 1) * chunkSize);
                                                    return std::reduce(sb, se);
                                                }));
            }

            std::vector<T> resValues{};
            for (auto &f : futures)
                resValues.emplace_back(f.get());

            return std::reduce(resValues.begin(), resValues.end());
        }
#endif
    } // namespace _Detail

    template <typename Iter>
    typename std::iterator_traits<Iter>::value_type Reduce(Iter beg, Iter end)
    {
#ifdef __ParallelUseTbb
        return _Detail::ReduceTbb(beg, end);
#elif defined(__ParallelUseOpenMP)
        return _Detail::ReduceOpenMP(beg, end);
#elif defined(__ParallelUseStb)
        return _Detail::ReduceStb(beg, end);
#elif defined(__ParallelUseSingleThread)
        return std::reduce(beg, end);
#else
        return std::reduce(std::execution::par_unseq, beg, end);
#endif
    }

    namespace _Detail
    {
#ifdef __ParallelUseTbb
        template <typename Iter>
        void ReverseTbb(Iter beg, Iter end)
        {
            const auto size = end - beg;
            if (size <= 1)
                return;

            const auto count = static_cast<std::size_t>(size) / 2;

            tbb::parallel_for(tbb::blocked_range<std::size_t>(0, count),
                              [=](tbb::blocked_range<std::size_t> &rng)
                              {
                                  for (auto i = rng.begin(); i != rng.end(); ++i)
                                  {
                                      using std::swap;
                                      swap(*(beg + i), *(beg + (size - 1 - i)));
                                  }
                              });
        }
#endif

#ifdef __ParallelUseOpenMP
        template <typename Iter>
        void ReverseOpenMP(Iter beg, Iter end)
        {
            const auto size = end - beg;
            if (size <= 1)
                return;

            const auto count = static_cast<std::size_t>(size) / 2;

#pragma omp parallel for
            for (std::size_t i = 0; i < count; ++i)
            {
                using std::swap;
                swap(*(beg + i), *(beg + (size - 1 - i)));
            }
        }
#endif

#ifdef __ParallelUseStb
        template <typename Iter>
        void ReverseStb(Iter beg, Iter end)
        {
            const auto size = end - beg;
            if (size <= 1)
                return;

            const auto count = static_cast<std::size_t>(size) / 2;

            const auto threadCount = std::min<std::size_t>(std::thread::hardware_concurrency(), count);
            const auto chunkSize = count / threadCount;

            std::vector<std::future<void>> futures{};
            for (std::size_t i = 0; i < threadCount; ++i)
            {
                futures.emplace_back(std::async(std::launch::async,
                                                [=]()
                                                {
                                                    const auto sb = i * chunkSize;
                                                    const auto se = ((i == threadCount - 1) ? count : (i + 1) * chunkSize);
                                                    for (std::size_t j = sb; j < se; ++j)
                                                    {
                                                        using std::swap;
                                                        swap(*(beg + j), *(beg + (size - 1 - j)));
                                                    }
                                                }));
            }
            for (auto &f : futures)
                f.get();
        }
#endif
    }

    template <typename Iter>
    void Reverse(Iter beg, Iter end)
    {
#ifdef __ParallelUseTbb
        _Detail::ReverseTbb(beg, end);
#elif defined(__ParallelUseOpenMP)
        _Detail::ReverseOpenMP(beg, end);
#elif defined(__ParallelUseStb)
        _Detail::ReverseStb(beg, end);
#elif defined(__ParallelUseSingleThread)
        std::reverse(beg, end);
#else
        return std::reverse(std::execution::par_unseq, beg, end);
#endif
    }
} // namespace Parallel
