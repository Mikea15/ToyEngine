#pragma once

#include "../Containers/ThreadSafeQueue.h"
#include "IBaseJob.h"

#include <memory>
#include <thread>
#include <algorithm>
#include <chrono>
#include <atomic>

#define MAX_THREADS 8u



class JobScheduler
{
public:
    static JobScheduler& GetInstance()
    {
        return m_instance;
    }

    JobScheduler()
    {
        
    }

    ~JobScheduler()
    {
        
    }

    void Init()
    {
        unsigned int availableThreads = std::min(std::thread::hardware_concurrency(), MAX_THREADS);
        for (size_t i = 0; i < availableThreads; i++)
        {
            std::thread t([&]() { WorkLoop(); });
            m_runningThreads.push_back(std::move(t));
        }
        m_running.store(true);
    }

    void Cleanup()
    {
        m_running.store(false);
        for (size_t i = 0; i < m_runningThreads.size(); i++)
        {
            m_runningThreads[i].join();
        }
    }

    void WorkLoop()
    {
        while (m_running.load())
        {
            if (!m_jobs.empty())
            {
                // grab a job and execute.
                IBaseJob* job{};
                if (m_jobs.try_pop(job))
                {
                    job->Execute();
                }
            }
            else
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
    }

    void AddJob(IBaseJob* job)
    {
        m_jobs.push(job);
    }

    bool HasJobs() { return !m_jobs.empty(); }


private:
    static JobScheduler m_instance;

    ThreadSafeQueue<IBaseJob*> m_jobs;
    std::vector<std::thread> m_runningThreads;
    std::atomic_bool m_running;

    int m_runningJobs;
};


