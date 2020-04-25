#pragma once

#include "Engine/State.h"

#include "OOP/Boid.h"
#include "Path.h"

class Game;

#if MULTITHREAD
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#endif

struct JobBlock
{
    size_t start;
    size_t end;
    std::vector<Boid> boids;
};

class BoidSystemState
    : public BaseState
{
public:
    BoidSystemState()
        : BaseState()
    {}

    ~BoidSystemState() override {};

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
#if USE_OCTREE
            m_octree = Octree(glm::vec3(0.0f), 50.0f);
            OcNode nodeData;
            for (size_t i = 0; i < ENTITY_COUNT; i++)
            {
                m_octree.Insert(m_wanderers[i].m_position, i);
            }
#endif

#if MULTITHREAD

            std::vector<Boid> m_wanderersSnapshot(m_wanderers);

            std::vector<std::thread> threads;

            std::atomic<int> finishedThreads{ 0 };
            const size_t groupSize = m_wanderers.size() / NUM_THREADS;

            std::vector<JobBlock> job;
            job.resize(NUM_THREADS);

            for (size_t t = 0; t < NUM_THREADS; ++t)
            {
                auto& tJob = job[t];

                tJob.start = t * groupSize;
                tJob.end = tJob.start + groupSize;
                tJob.boids = std::vector<Boid>(m_wanderers.begin() + (groupSize * t), m_wanderers.begin() + (groupSize * t) + groupSize);


                std::thread thr([&]() {
                    for (size_t i = 0; i < tJob.boids.size(); i++)
                    {
                        tJob.boids[i].UpdateTargets();
                        glm::vec3 force = tJob.boids[i].CalcSteeringBehavior(m_wanderersSnapshot, neighborIndices);
                        tJob.boids[i].UpdatePosition(deltaTime, force);
                    }

                    // no more jobs.
                    {
                        std::lock_guard<std::mutex> lock(m_mutex);
                        finishedThreads++;

                        for (size_t i = tJob.start; i < tJob.end; i++)
                        {
                            m_wanderers[i] = tJob.boids[i - tJob.start];
                        }

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
#else
            for (size_t i = 0; i < ENTITY_COUNT; i++)
            {
#if USE_OCTREE
                AABB searchAabb = AABB(m_wanderers[i].m_position, m_wanderers[i].m_properties->m_neighborRange);
                neighborResult.clear();

                m_octree.Search(searchAabb, neighborResult);
                neighborIndices.clear();
                for (const OcNode& node : neighborResult)
                {
                    if (node.m_storeIndex == i) 
                    {
                        continue;
                    }
                    neighborIndices.insert(node.m_storeIndex);
                }
#endif // USE_OCTREE
                // Agent Core Loop
                {
                    m_wanderers[i].UpdateTargets();

                    glm::vec3 force = m_wanderers[i].CalcSteeringBehavior(m_wanderers, neighborIndices);
                    m_wanderers[i].UpdatePosition(deltaTime, force);
                }
                m_wanderers[i].DrawDebug();
            }
#if USE_OCTREE
            m_octree.DebugDraw();
#endif
#endif // MULTITHREAD
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

#if MULTITHREAD
    std::mutex m_mutex;
    std::condition_variable m_cvar;
#endif
};
