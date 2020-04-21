#pragma once

#include "Engine/State.h"

#include "OOP/Boid.h"
#include "Path.h"

class Game;

#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>

#if _DEBUG
#define ENTITY_COUNT 400
#else
#define ENTITY_COUNT 2000
#endif

struct JobBlock
{
    size_t start;
    size_t end;
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

        neighborIndices = VectorContainer<size_t>(ENTITY_COUNT);

        DebugDraw::Init();
    };

    void HandleInput(SDL_Event* event) override
    {
        BaseState::HandleInput(event);
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


        {
            std::vector<Boid> m_wanderersSnapshot(m_wanderers);

            std::vector<std::thread> threads;

            std::atomic<int> finishedThreads{ 0 };
            size_t groupSize = m_wanderers.size() / NUM_THREADS;

            for (size_t t = 0; t < NUM_THREADS; ++t)
            {
                JobBlock job;
                job.start = t * groupSize;
                job.end = job.start + groupSize;

                std::thread thr([&]() {
                    for (size_t i = job.start; i < job.end; i++)
                    {
                        m_wanderers[i].UpdateTargets();
                        glm::vec3 force = m_wanderers[i].CalcSteeringBehavior(m_wanderersSnapshot, neighborIndices);
                        m_wanderers[i].UpdatePosition(deltaTime, force);
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

            for (size_t i = 0; i < ENTITY_COUNT; i++)
            {
                m_wanderers[i].DrawDebug();
            }
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
    };

    void Render(float alpha = 1.0f) override
    {
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

    Octree m_octree;
    std::vector<OcNode> neighborResult;
    VectorContainer<size_t> neighborIndices;

    std::mutex m_mutex;
    std::condition_variable m_cvar;
};
