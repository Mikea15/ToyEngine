#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>

template<typename T>
class ThreadSafeQueue
{
public:
    ThreadSafeQueue() {}
    ThreadSafeQueue(const ThreadSafeQueue& copy)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_data = copy.m_data;
    }

    void Push(T val)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_data.push(val);
        m_cvar.notify_one();
    }

    void WaitAndPop(T& val)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cvar.wait([this]() { return !m_data.empty(); });

        val = m_data.front();
        m_data.pop();
    }

    std::shared_ptr<T> WaitAndPop()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cvar.wait([this]() { return !m_data.empty(); });

        std::shared_ptr<T> val(std::make_shared<T>(m_data.front()));
        m_data.pop();

        return val;
    }

    bool TryPop(T& val)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_data.empty())
        {
            return false;
        }

        val = m_data.front();
        m_data.pop();

        return true;
    }

    std::shared_ptr<T> TryPop()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_data.empty())
        {
            return std::shared_ptr<T>();
        }

        std::shared_ptr<T> val(std::make_shared<T>(m_data.front()));
        m_data.pop();

        return val;
    }

    bool Empty()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_data.empty();
    }

    unsigned int Size()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return static_cast<unsigned int>(m_data.size());
    }

private:
    std::mutex m_mutex;
    std::queue<T> m_data;
    std::condition_variable m_cvar;
};