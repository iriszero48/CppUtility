#pragma once

#include <condition_variable>
#include <mutex>
#include <list>
#include <optional>
#include <atomic>

#ifdef __cpp_lib_coroutine
#include <coroutine>
#endif

namespace CuThread
{
    constexpr int Version[]{1, 0, 0, 0};

    template <typename Func>
    struct Synchronize
    {
        std::mutex Mtx{};
        Func F;

        explicit Synchronize(Func func) : F(std::move(func)) {}

        template <typename... Args>
        decltype(auto) operator()(Args &&...args)
        {
            std::lock_guard lock(Mtx);
            return F(std::forward<Args>(args)...);
        }
    };

    constexpr size_t Dynamics = ~size_t{0};

    template <typename T, size_t Limit = 0>
    class Channel
    {
    public:
        std::atomic_size_t DynLimit = 0;

        void Write(T &&data)
        {
            std::unique_lock lock(mtx);
            if constexpr (Limit)
                writeCond.wait(lock, [&]()
                               {
                if constexpr (Limit == Dynamics)
                {
                    return buffer.size() < DynLimit;
                }
                else
                {
                    return buffer.size() < Limit;
                } });
            buffer.push_back(std::move(data));
            lock.unlock();
            readCond.notify_all();
        }

        template <typename... Args>
        void Emplace(Args &&...args)
        {
            T val{std::forward<Args>(args)...};
            Write(std::move(val));
        }

        T Read()
        {
            std::unique_lock lock(mtx);
            readCond.wait(lock, [&]()
                          { return !buffer.empty(); });
            auto item = std::move(buffer.front());
            buffer.pop_front();
            lock.unlock();
            if constexpr (Limit)
                writeCond.notify_all();
            return std::move(item);
        }

        [[nodiscard]] auto Length() const
        {
            return buffer.size();
        }

        [[nodiscard]] auto Empty() const
        {
            return buffer.empty();
        }

    private:
        std::list<T> buffer{};
        std::mutex mtx{};
        std::condition_variable readCond{};
        std::condition_variable writeCond{};
    };

    class Semaphore
    {
    public:
	    explicit Semaphore(const size_t count) : count(count) {}

        void Release()
        {
            std::unique_lock lock(mtx);
            count++;
            lock.unlock();
            cv.notify_one();
        }

        void WaitOne()
        {
            std::unique_lock lock(mtx);
            cv.wait(lock, [&]() { return count != 0; });
            count--;
        }

        void lock()
	    {
            WaitOne();
	    }

        void unlock()
	    {
            Release();
	    }

    private:
        std::mutex mtx;
        std::condition_variable cv;
        size_t count;
    };

    template <typename T>
    class Stack
    {
        struct Node
        {
            T Data;
            Node *Next;
        };

        struct TagNode
        {
            int Tag;
            Node *Head;
        };

        std::atomic<TagNode> head = TagNode{0, nullptr};

    public:
        void Push(const T &value)
        {
            TagNode next = TagNode{};
            TagNode orig = head.load(std::memory_order_relaxed);
            Node *node = new Node{};
            node->Data = value;
            do
            {
                node->Next = orig.Head;
                next.Head = node;
                next.Tag = orig.Tag + 1;
            } while (!head.compare_exchange_weak(orig, next,
                                                 std::memory_order_release,
                                                 std::memory_order_relaxed));
        }

        std::optional<T> Pop()
        {
            TagNode next = TagNode{};
            TagNode orig = head.load(std::memory_order_relaxed);
            do
            {
                if (orig.Head == nullptr)
                    return std::nullopt;
                next.Head = orig.Head->Next;
                next.Tag = orig.Tag + 1;
            } while (!head.compare_exchange_weak(orig, next,
                                                 std::memory_order_release,
                                                 std::memory_order_relaxed));
            const auto res = orig.Head->Data;
            delete orig.Head;
            return res;
        }
    };

#ifdef __cpp_lib_coroutine
    template <std::movable T>
    class Generator
    {
    public:
        struct promise_type
        {
            Generator get_return_object()
            {
                return Generator{Handle::from_promise(*this)};
            }
            static std::suspend_always initial_suspend() noexcept
            {
                return {};
            }
            static std::suspend_always final_suspend() noexcept
            {
                return {};
            }

            template <std::convertible_to<T> ValueType>
            std::suspend_always yield_value(ValueType &&value) noexcept
            {
                current_value = std::move(value);
                return {};
            }
            void await_transform() = delete;
            [[noreturn]] static void unhandled_exception() { throw; }
            void return_void()
            {
                current_value.reset();
            }

            std::optional<T> current_value{};
        };

        using Handle = std::coroutine_handle<promise_type>;

        explicit Generator(const Handle handle) : m_coroutine{handle}
        {
        }

        Generator() = default;
        ~Generator()
        {
            if (m_coroutine)
            {
                m_coroutine.destroy();
            }
        }

        Generator(const Generator &) = delete;
        Generator &operator=(const Generator &) = delete;

        Generator(Generator &&other) noexcept : m_coroutine{other.m_coroutine}
        {
            other.m_coroutine = {};
        }
        Generator &operator=(Generator &&other) noexcept
        {
            if (this != &other)
            {
                if (m_coroutine)
                {
                    m_coroutine.destroy();
                }
                m_coroutine = other.m_coroutine;
                other.m_coroutine = {};
            }
            return *this;
        }

        class Iter
        {
        public:
            using iterator_category = std::input_iterator_tag;
            using value_type = T;
            using difference_type = std::int64_t;
            using pointer = T *;
            using reference = T &;

            Iter &operator++()
            {
                m_coroutine.resume();
                return *this;
            }

            const T &operator*() const
            {
                return *m_coroutine.promise().current_value;
            }

            bool operator==(std::default_sentinel_t) const
            {
                return !m_coroutine || !m_coroutine.promise().current_value.has_value();
            }

            bool operator==(const Iter &it) const
            {
                bool lv = false;
                if (m_coroutine)
                    lv = true;
                if (lv)
                    lv = m_coroutine.promise().current_value.has_value();

                bool rv = false;
                if (it.m_coroutine)
                    rv = true;
                if (rv)
                    rv = it.m_coroutine.promise().current_value.has_value();

                if (lv && rv)
                {
                    return *m_coroutine.promise().current_value == *it.m_coroutine.promise().current_value;
                }

                return false;
            }

            bool operator!=(const Iter &it) const
            {
                return !(*this == it);
            }

            Iter() = default;
            explicit Iter(const Handle coroutine) : m_coroutine{coroutine} {}

        private:
            Handle m_coroutine{};
        };

        Iter begin()
        {
            if (m_coroutine)
            {
                m_coroutine.resume();
            }
            return Iter{m_coroutine};
        }

        Iter end()
        {
            return {};
        }

    private:
        Handle m_coroutine;
    };
#endif
}
