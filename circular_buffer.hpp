#ifndef CB_HPP_2024_
#define CB_HPP_2024_

#include <array>
#include <chrono>
#include <mutex>

template <typename elem_type, size_t Size> class circular_buffer
{
  public:
    circular_buffer()
    {
        static_assert(Size > 0, "Buffer without memory");
    }
    circular_buffer(circular_buffer const &other)
    {
        std::lock_guard<std::mutex> lk{other.m_mut};
        m_mem = other.m_mem;
        m_head = other.m_head;
        m_tail = other.m_tail;
        m_elements_count = other.m_elements_count;
    }
    circular_buffer &operator=(circular_buffer const &) = delete;

    void put(elem_type elem)
    {
        std::lock_guard<std::mutex> lk{m_mut};
        if (m_elements_count == Size)
            ++m_tail %= Size;
        m_mem[m_head++] = std::move(elem);
        m_head %= Size;
        m_elements_count = std::min(++m_elements_count, Size);
        m_cv.notify_one();
    }

    // Blocking
    bool get(elem_type &elem, std::chrono::duration<long, std::milli> const timeout = time_limit_ms)
    {
        std::unique_lock<std::mutex> lk{m_mut};
        bool const res = m_cv.wait_for(lk, timeout, [this] { return m_elements_count > 0; });
        if (!res)
            return false;

        elem = m_mem[m_tail];
        adjust_tail();
        return true;
    }

    // Non Blocking
    bool try_get(elem_type &elem)
    {
        std::lock_guard<std::mutex> lk{m_mut};
        if (!m_elements_count)
            return false;

        elem = m_mem[m_tail];
        adjust_tail();
        return true;
    }

    // Blocking
    std::shared_ptr<elem_type> get(std::chrono::duration<long, std::milli> const timeout = time_limit_ms)
    {
        std::unique_lock<std::mutex> lk{m_mut};
        bool const res = m_cv.wait_for(lk, timeout, [this] { return m_elements_count > 0; });

        if (!res)
            return {}; // Default value as failure

        auto item = std::make_shared<elem_type>(m_mem[m_tail]);
        adjust_tail();
        return item;
    }

    // Non Blocking
    std::shared_ptr<elem_type> try_get()
    {
        std::lock_guard<std::mutex> lk{m_mut};
        if (!m_elements_count)
            return {};

        auto item = std::make_shared<elem_type>(m_mem[m_tail]);
        adjust_tail();
        return item;
    }

    bool empty() const
    {
        std::lock_guard<std::mutex> lk{m_mut};
        return m_elements_count == 0;
    }

    bool full() const
    {
        std::lock_guard<std::mutex> lk{m_mut};
        return m_elements_count == Size;
    }

    size_t size() const
    {
        std::lock_guard<std::mutex> lk{m_mut};
        return m_elements_count;
    }

  private:
    static constexpr std::chrono::duration<int, std::milli> time_limit_ms{std::chrono::milliseconds{5}};

    void adjust_tail()
    {
        if (m_elements_count)
        {
            ++m_tail %= Size;
            --m_elements_count;
        }
    }

    std::condition_variable m_cv;
    mutable std::mutex m_mut;
    std::array<elem_type, Size> m_mem{};
    typename std::array<elem_type, Size>::size_type m_head{};
    typename std::array<elem_type, Size>::size_type m_tail{};
    size_t m_elements_count{};
};

#endif