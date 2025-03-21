#ifndef _COMMON_SOURCE_CPP_THREADSAFE_QUEUE_H
#define _COMMON_SOURCE_CPP_THREADSAFE_QUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include <initializer_list>

template<typename T, size_t MAX_BUFFER_DEPTH = 10000>
class threadsafe_queue
{
private:
    mutable std::mutex mut;
    mutable std::condition_variable data_cond;
    using queue_type = std::queue<T>;
    queue_type data_queue;

public:
    using value_type = typename queue_type::value_type;
    using container_type = typename queue_type::container_type;
    threadsafe_queue() = default;
    threadsafe_queue(const threadsafe_queue&) = delete;
    threadsafe_queue& operator=(const threadsafe_queue&) = delete;

    template<typename _InputIterater>
    threadsafe_queue(_InputIterater first, _InputIterater last)
    {
        for (auto iter = first; iter != last; ++iter)
        {
            data_queue.push(*iter);
        }
    }
    explicit threadsafe_queue(const container_type& c): data_queue(c) {}

    threadsafe_queue(std::initializer_list<value_type> list): threadsafe_queue(list.begin(), list.end()) {}

    //void push(const value_type& new_value)
    //{
    //    std::lock_guard<std::mutex> lk(mut);
    //    if (data_queue.size() > MAX_BUFFER_DEPTH)
    //        return;
    //    data_queue.push(new_value);
    //    data_cond.notify_one();
    //}

    void push(value_type&& new_value)
    {
        std::lock_guard<std::mutex> lk(mut);
        if (data_queue.size() > MAX_BUFFER_DEPTH)
            return;
        data_queue.push(std::move(new_value));
        data_cond.notify_one();
    }

    value_type wait_and_pop()
    {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk, [this] { return !data_queue.empty(); });
        auto value = std::move(data_queue.front());
        data_queue.pop();
        return value;
    }

    bool try_pop(value_type& value)
    {
        std::lock_guard<std::mutex> lk(mut);
        if (data_queue.empty())
            return false;
        value = std::move(data_queue.front());
        data_queue.pop();
        return true;
    }

    void clear()
    {
        std::lock_guard<std::mutex> lk(mut);
        data_queue = queue_type();
    }

    void notify()
    {
        std::lock_guard<std::mutex> lk(mut);
        data_cond.notify_all();
    }

    auto empty() const -> decltype(data_queue.empty())
    {
        std::lock_guard<std::mutex> lk(mut);
        return data_queue.empty();
    }

    auto size() const -> decltype(data_queue.size())
    {
        std::lock_guard<std::mutex> lk(mut);
        return data_queue.size();
    }
};

#endif
