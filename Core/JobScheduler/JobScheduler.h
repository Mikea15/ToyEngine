#pragma once

#include "../Containers/ThreadSafeQueue.h"
#include "IBaseJob.h"

#include <memory>
#include <thread>
#include <algorithm>
#include <chrono>
#include <atomic>
#include <set>
#include <map>

#define MAX_THREADS 8u

#define ENABLE_MT_JS 0

class JobScheduler
{
public:
	using JobBehavior = std::function<void(int)>;

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
#if ENABLE_MT_JS
		unsigned int availableThreads = std::min(std::thread::hardware_concurrency(), MAX_THREADS);
		for (size_t i = 0; i < availableThreads; i++)
		{
			std::thread t([&]() { WorkLoop(); });
			m_runningThreads.push_back(std::move(t));
		}
		m_running.store(true);
#endif
	}

	void Cleanup()
	{
#if ENABLE_MT_JS
		m_running.store(false);
		for (size_t i = 0; i < m_runningThreads.size(); i++)
		{
			m_runningThreads[i].join();
		}
#endif
	}

	void WorkLoop()
	{
#if ENABLE_MT_JS
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
#endif
	}

	void AddJob(IBaseJob* job)
	{
#if ENABLE_MT_JS
		m_jobs.push(job);
#endif
	}

	bool HasJobs()
	{
#if ENABLE_MT_JS
		return !m_jobs.empty();
#endif
		return !behaviorRecords.empty();
	}

	size_t Size() const { return behaviorRecords.size(); }


	void AddBehavior(JobBehavior function, int frequency, int phase)
	{
		BehaviorRecord br;
		br.fn = function;
		br.frequency = frequency;
		br.phase = phase;
		behaviorRecords.push_back(br);
	}

	void Run(int timeToRun)
	{
		++frame;

		std::vector<BehaviorRecord> toRunThisFrame;
		for (auto it = behaviorRecords.begin(); it != behaviorRecords.end();)
		{
			if (it->frequency % (frame + it->phase)) 
			{
				toRunThisFrame.push_back(*it);
				it = behaviorRecords.erase(it);
			}
			else
			{
				++it;
			}
		}

		auto currentTime = std::chrono::high_resolution_clock::now();
		size_t behaviorsToRun = toRunThisFrame.size();
		size_t ranBehaviors = 0u;
		for (BehaviorRecord br : toRunThisFrame)
		{
			auto lastTime = currentTime;
			currentTime = std::chrono::high_resolution_clock::now();
			auto timeUsed = currentTime - lastTime;

			timeToRun -= std::chrono::duration_cast<std::chrono::duration<float>>(timeUsed).count();
			int availableTime = timeToRun / (behaviorsToRun - ranBehaviors);

			br.fn(availableTime);
			++ranBehaviors;
		}
	}

private:
	static JobScheduler m_instance;

	struct BehaviorRecord
	{
		JobBehavior fn;
		size_t frequency;
		size_t phase;
	};

	// frequency / sets
	std::vector<BehaviorRecord> behaviorRecords;

	size_t frame = 0u;

#if ENABLE_MT_JS
	ThreadSafeQueue<IBaseJob*> m_jobs;
	std::vector<std::thread> m_runningThreads;
	std::atomic_bool m_running;
#endif



	int m_runningJobs;
};


