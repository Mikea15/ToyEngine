#pragma once

#include "Engine/State.h"

#include "OOP/Boid.h"
#include "Path.h"

#include "Engine/Systems/KDTree.h"

class Game;

#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>

#include "Engine/Core/ThreadSafeQueue.h"

struct JobBlock
{
    size_t start;
    size_t end;
    std::vector<Boid> boids;
};

struct AsyncJob
{
    size_t index;
    Boid copy;
};

struct BoidJob
{
    // input
    std::vector<size_t> neighborIndices;
    size_t index;
    float deltatime;

    // in/out
    Boid agent;
};


class ThreadedState
    : public BaseState
{
public:
    ThreadedState()
        : BaseState()
    {}

    ~ThreadedState() override {};

    void Init(Game* game) override
    {
        BaseState::Init(game);

        // configure camera
        m_camera = FlyCamera(glm::vec3(0.0f, 25.0f, 25.0f));
        m_camera.SetPerspective(glm::radians(90.0f),
            static_cast<float>(m_renderer->GetRenderWidth() / m_renderer->GetRenderHeight()),
            0.01f, 200.0f);

        m_renderer->SetCamera(&m_camera);

        m_viewGrid = ViewportGrid(100, 100, 100, 100);

        m_path = Path({
            glm::vec3(0.0f, 0.0f, 40.0f),
            glm::vec3(13.5f, 0.0f, 25.0f),
            glm::vec3(25.0f, 0.0f, 10.0f),
            glm::vec3(40.0f, 0.0f, 0.0f),
            glm::vec3(45.0f, 0.0f, -25.0f),
            glm::vec3(25.0f, 0.0f, -45.0f),
            glm::vec3(10.0f, 0.0f, -25.0f),
            glm::vec3(0.0f, 0.0f, -10.0f),
            glm::vec3(-10.0f, 0.0f, -25.0f),
            glm::vec3(-25.0f, 0.0f, -25.0f),
            glm::vec3(-45.0f, 0.0f, 0.0f),
            glm::vec3(-25.0f, 0.0f, 25.0f)
            });

        m_path2 = Path({
            glm::vec3(-25.0f, 5.0f, 25.0f),
            glm::vec3(-45.0f, 5.0f, 0.0f),
            glm::vec3(-25.0f, 5.0f, -25.0f),
            glm::vec3(-10.0f, 5.0f, -25.0f),
            glm::vec3(0.0f, 5.0f, -10.0f),
            glm::vec3(10.0f, 5.0f, -25.0f),
            glm::vec3(25.0f, 5.0f, -45.0f),
            glm::vec3(45.0f, 5.0f, -25.0f),
            glm::vec3(40.0f, 5.0f, 0.0f),
            glm::vec3(25.0f, 5.0f, 10.0f),
            glm::vec3(13.5f, 5.0f, 25.0f),
            glm::vec3(0.0f, 5.0f, 40.0f)
            });

        for (size_t i = 0; i < ENTITY_COUNT; i++)
        {
            auto b = Boid(&m_sharedBoidProperties);
            auto features =
                eSeek |
                eAlignment |
                eSeparation |
                eCohesion |
                eWallLimits;

            b.SetFeature(features);

            b.m_position = glm::vec3(
                MathUtils::Rand(-50.0f, 50.0f),
                MathUtils::Rand(-50.0f, 50.0f),
                MathUtils::Rand(-50.0f, 50.0f)
            );

            if (MathUtils::Rand01() > 0.5f)
            {
                b.SetTarget(&m_simplePathFollower);
            }
            else
            {
                b.SetTarget(&m_simplePathFollower2);
            }

            m_wanderers.push_back(b);
        }

        m_simplePathFollower.SetFeature(eSeek);
        m_simplePathFollower2.SetFeature(eSeek);

        for (size_t i = 0; i < 30; i++)
        {
            randomPoints.push_back(
                MathUtils::RandomInUnitSphere() * 10.0f
            );
        }

#if USE_THREAD_JOBS
        m_isRunning.store(true);
        for (size_t i = 0; i < NUM_THREADS; ++i)
        {
            m_workers.push_back(std::thread([&]() { WorkerThreadLoop(); }));
        }
#endif

        DebugDraw::Init();
    };

    void Update(float deltaTime) override
    {
        BaseState::Update(deltaTime);

        // Debug
        DebugDraw::Clear();

        glm::mat4 viewProj = m_camera.GetViewProjection();
        DebugDraw::Update(viewProj);

        m_viewGrid.Draw();
        DebugDraw::AddAABB(glm::vec3(0.0f) - glm::vec3(50.0f, 0.0f, 50.0f),
            glm::vec3(0.0f) + glm::vec3(50.0f, 0.0f, 50.0f));

        AABB limits = AABB(glm::vec3(0.0f, 25.0f, 0.0f), 50);
        DebugDraw::AddAABB(limits.GetMin(), limits.GetMax());

#if USE_OCTREE
        PopulateOctree();
#elif USE_KDTREE
        PopulateKDTree();
#endif

#if USE_THREAD
        std::vector<std::thread> threads;
        std::atomic<int> finishedThreads{ 0 };
        const size_t groupSize = m_wanderers.size() / NUM_THREADS;
        size_t rest = m_wanderers.size() % NUM_THREADS;

        std::vector<JobBlock> job;
        job.resize(NUM_THREADS);

        size_t groupIndex = 0;

        for (size_t t = 0; t < NUM_THREADS; ++t)
        {
            auto& tJob = job[t];

            tJob.start = t * groupSize;
            tJob.end = tJob.start + groupSize;
            if (t == NUM_THREADS - 1) {
                tJob.end += rest;
            }
            tJob.boids = std::vector<Boid>(m_wanderers.begin() + (tJob.start), m_wanderers.begin() + tJob.end);

            std::thread thr([&]() {
                size_t boidsize = tJob.boids.size();
                for (size_t i = 0; i < boidsize; i++)
                {
                    tJob.boids[i].UpdateTargets();
                    glm::vec3 force = tJob.boids[i].CalcSteeringBehavior(m_wanderers, neighborIndices);
                    tJob.boids[i].UpdatePosition(deltaTime, force);
                }

                // no more jobs.
                {
                    std::lock_guard<std::mutex> lock(m_mutex);
                    finishedThreads++;
                    m_cvar.notify_one();
                }
                });
            threads.push_back(std::move(thr));
        }

        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cvar.wait(lock, [&finishedThreads] {
                return finishedThreads == NUM_THREADS;
                });
        }

        for (std::thread& t : threads)
        {
            t.join();
        }

        for (size_t t = 0; t < NUM_THREADS; t++)
        {
            for (size_t i = job[t].start; i < job[t].end; i++)
            {
                m_wanderers[i] = job[t].boids[i - job[t].start];
            }
        }
#elif USE_ASYNC
        std::vector<std::future<AsyncJob>> futures;

        for (size_t i = 0; i < ENTITY_COUNT; i++)
        {
            auto future = std::async(std::launch::async, [deltaTime, i, this]() {
                m_wanderers[i].UpdateTargets();
                glm::vec3 force = m_wanderers[i].CalcSteeringBehavior(m_wanderers, neighborIndices);
                m_wanderers[i].UpdatePosition(deltaTime, force);

                AsyncJob aj;
                aj.index = i;
                aj.copy = m_wanderers[i];
                return aj;
                });

            {
                std::lock_guard<std::mutex> lock(m_mutex);
                futures.push_back(std::move(future));
            }
        }

        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cvar.wait(lock, [&futures] {
                return futures.size() == ENTITY_COUNT;
                });
        }

        for (std::future<AsyncJob>& aj : futures)
        {
            //aj.wait();

            AsyncJob result = aj.get();
            m_wanderers[result.index] = result.copy;
        }

#elif USE_THREAD_JOBS
        // prepare jobs
        for (size_t i = 0; i < ENTITY_COUNT; i++)
        {
            BoidJob job;
            job.agent = m_wanderers[i];
            job.deltatime = deltaTime;
            job.index = i;
            job.neighborIndices = neighborIndices;

            m_boidJobQ.push(job);
        }

        {
            std::unique_lock<std::mutex> lock(m_jobFinishedMutex);
            m_cvarNotifJobsDone.wait(lock, [&]() { return m_finishedJobs.size() == ENTITY_COUNT; });
        }

        for (const BoidJob& b : m_finishedJobs)
        {
            m_wanderers[b.index] = b.agent;
        }

        m_finishedJobs.clear();
#endif
        for (size_t i = 0; i < ENTITY_COUNT; i++)
        {
            m_wanderers[i].DrawDebug();
        }

        neighborIndices.clear();

        {
            m_path.UpdatePath(m_simplePathFollower.m_position);
            m_path.DebugDraw();

            m_simplePathFollower.SetTarget(m_path.GetCurrentGoal());
            m_simplePathFollower.FullUpdate(deltaTime, m_wanderers, neighborIndices);
            m_simplePathFollower.DrawDebug();
        }

        {
            m_path2.UpdatePath(m_simplePathFollower2.m_position);
            m_path2.DebugDraw();

            m_simplePathFollower2.SetTarget(m_path2.GetCurrentGoal());
            m_simplePathFollower2.FullUpdate(deltaTime, m_wanderers, neighborIndices);
            m_simplePathFollower2.DrawDebug();
        }

    }

    void PopulateOctree()
    {
        m_octree = AABBOctree(glm::vec3(0.0f), 50.0f);
        for (size_t i = 0; i < ENTITY_COUNT; i++)
        {
            m_octree.Insert(m_wanderers[i].m_position, i);
        }
    }

    void QueryOctree(glm::vec3 pos, float range, size_t agentIndex)
    {
        AABB searchAabb = AABB(pos, range);

        neighborResult.clear();
        m_octree.Search(searchAabb, neighborResult);

#if USE_OCTREE_PRUNE_BY_DIST
        neighborResult.erase(std::remove_if(neighborResult.begin(), neighborResult.end(), [&](const OcNode& n) {
            return glm::length2(pos - m_wanderers[n.m_data].m_position) > range * range;
            }), neighborResult.end());
#endif

        neighborIndices.clear();
        for (const OcNode& node : neighborResult)
        {
            if (node.m_data == agentIndex) { continue; }
            neighborIndices.emplace_back(node.m_data);
        }
    }

    void PopulateKDTree()
    {
        std::vector<kdtree::NodeContent> content;
        for (size_t i = 0; i < ENTITY_COUNT; i++)
        {
            content.push_back({ m_wanderers[i].m_position, i });
        }

        m_kdtree = kdtree(content);
    }

    void QueryKDTree(glm::vec3 pos, float range, size_t agentIndex)
    {
        auto result = m_kdtree.nearest(pos, range);
        neighborIndices.clear();
        for (const kdtree::NodeContent& node : result)
        {
            if (node.second == agentIndex) continue;
            neighborIndices.emplace_back(node.second);
        }
    }

#if USE_THREAD_JOBS
    void WorkerThreadLoop()
    {
        while (m_isRunning.load())
        {
            BoidJob job;
            if (m_boidJobQ.try_pop(job))
            {
                job.agent.UpdateTargets();
                glm::vec3 force = job.agent.CalcSteeringBehavior(m_wanderers, neighborIndices);
                job.agent.UpdatePosition(job.deltatime, force);

                {
                    std::scoped_lock<std::mutex> lock(m_jobFinishedMutex);
                    m_finishedJobs.push_back(job);
                    m_cvarNotifJobsDone.notify_one();
                }
            }
            else
            {
                std::this_thread::yield();
                // std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }
    }
#endif

    void Render(float alpha = 1.0f) override
    {
#if 0
        for (SceneNode* node : m_sceneNodes)
        {
            m_renderer->PushRender(node);
        }
#endif
        m_renderer->RenderPushedCommands();

        DebugDraw::AddPosition(glm::vec3(0.0f), 0.5f);

        bool x_ray = false;
        DebugDraw::Draw(x_ray);
    };

    void RenderUI()
    {
        BaseState::RenderUI();

        Debug::ShowPanel(m_sharedBoidProperties);
    };

    void Cleanup() override
    {
#if USE_THREAD_JOBS
        m_isRunning.store(false);
        for (size_t i = 0; i < NUM_THREADS; ++i)
        {
            m_workers[i].join();
        }
#endif
        BaseState::Cleanup();
    };

private:
    ViewportGrid m_viewGrid;

    Boid m_simplePathFollower;
    Boid m_simplePathFollower2;

    Properties m_sharedBoidProperties;
    std::vector<Boid> m_wanderers;

    Path m_path;
    Path m_path2;

    AABBOctree m_octree;
    std::vector<OcNode> neighborResult;
    std::vector<size_t> neighborIndices;

    kdtree m_kdtree;
    std::vector<glm::vec3> randomPoints;

    std::mutex m_mutex;
    std::condition_variable m_cvar;

    // threaded. jobs
    std::atomic_bool m_isRunning;
    std::vector<std::thread> m_workers;

    ThreadSafeQueue<BoidJob> m_boidJobQ;

    std::mutex m_jobFinishedMutex;
    std::vector<BoidJob> m_finishedJobs;

    std::condition_variable m_cvarNotifJobsDone;
};
