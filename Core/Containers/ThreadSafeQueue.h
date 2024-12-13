#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include "../CustomMutex.h"

#define USE_CRIT 0

template<typename T, typename Mutex = std::mutex>
class ThreadSafeQueue
{
public:
    ThreadSafeQueue() {}
    ThreadSafeQueue(const ThreadSafeQueue& copy)
    {
#if USE_CRIT
        mutex.lock();
#else
        std::lock_guard<std::mutex> lock(m_mutex);
#endif
        m_data = copy.m_data;
        m_size = copy.m_size;
#if USE_CRIT
        mutex.unlock();
#endif
    }

    void push(T val)
    {
#if USE_CRIT
        mutex.lock();
#else
        std::lock_guard<std::mutex> lock(m_mutex);
#endif
        m_data.push(val);
        ++m_size;
        m_cvar.notify_one();
#if USE_CRIT
        mutex.unlock();
#endif
    }

    void wait_pop(T& val)
    {
#if USE_CRIT
        mutex.lock();
#else
        std::lock_guard<std::mutex> lock(m_mutex);
#endif
        m_cvar.wait([this]() { return m_size > 0; });

        val = m_data.front();
        m_data.pop();
        --m_size;
#if USE_CRIT
        mutex.unlock();
#endif
    }

    std::shared_ptr<T> wait_pop()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cvar.wait([this]() { return !m_data.empty(); });

        std::shared_ptr<T> val(std::make_shared<T>(m_data.front()));
        m_data.pop();
        --m_size;
#if USE_CRIT
        mutex.unlock();
#endif
        return val;
    }

    bool try_pop(T& val)
    {
        if (!m_size) { return false; }
#if USE_CRIT
        mutex.lock();
#else
        std::lock_guard<std::mutex> lock(m_mutex);
#endif

        val = m_data.front();
        m_data.pop();
        --m_size;

#if USE_CRIT
        mutex.unlock();
#endif
        return true;
    }

    std::shared_ptr<T> try_pop()
    {
        if (m_size == 0) { return std::shared_ptr<T>(); }
#if USE_CRIT
        mutex.lock();
#else
        std::lock_guard<std::mutex> lock(m_mutex);
#endif
        std::shared_ptr<T> val(std::make_shared<T>(m_data.front()));
        m_data.pop();
        --m_size;
#if USE_CRIT
        mutex.unlock();
#endif
        return val;
    }

    bool empty()
    {
        return m_size == 0;
#if 0
#if USE_CRIT
        mutex.lock();
#else
        std::lock_guard<std::mutex> lock(m_mutex);
#endif
        const bool isEmpty = m_data.empty();
#if USE_CRIT
        mutex.unlock();
#endif
        return isEmpty;
#endif
    }

    unsigned int size()
    {
        return m_size;
#if 0
#if USE_CRIT
        mutex.lock();
#else
        std::lock_guard<std::mutex> lock(m_mutex);
#endif
        const unsigned int s = static_cast<unsigned int>(m_data.size());

#if USE_CRIT
        mutex.unlock();
#endif
        return s;
#endif
    }

private:
    CustomMutex mutex;
    std::mutex m_mutex;
    std::queue<T> m_data;
    std::condition_variable m_cvar;

    unsigned int m_size = 0;
};