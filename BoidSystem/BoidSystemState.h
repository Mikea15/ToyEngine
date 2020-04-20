#pragma once

#include "Engine/State.h"

#include "OOP/Boid.h"
#include "Path.h"

class Game;

#if _DEBUG
#define ENTITY_COUNT 200
#else
#define ENTITY_COUNT 2000
#endif


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

        m_sphere = new Sphere(32, 32);
        Material* defaultForwardMat = m_renderer->CreateMaterial("default-fwd");

        m_viewGrid = ViewportGrid(100, 100, 100, 100);

        int segments = 10;
        float angleIncrement = 360.0f / segments;
        for (size_t i = 0; i < segments; i++)
        {
            float angle = static_cast<float>(i) * angleIncrement / 180 * 3.14;
            m_randomPositions.push_back(
                glm::normalize(
                    glm::vec3( sin(angle), 0.0f, cos(angle) )
                ) * 10.0f
            );
        }

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
                Boid::eSeek | 
                Boid::eAlignment | 
                Boid::eSeparation | 
                Boid::eCohesion | 
                Boid::eWallLimits;

            b.SetFeature(features);

            // b.m_maxAccelerationForce = MathUtils::Rand01() * 0.1f;
            // b.m_maxVelocity = MathUtils::Rand01() * 5.0f;
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

        m_simpleFollower.SetFeature(Boid::eSeek);
        m_simplePathFollower.SetFeature(Boid::eSeek);
        m_simplePathFollower2.SetFeature(Boid::eSeek);

        m_simpleArriver.SetFeature(Boid::eArrive);

        m_simpleFlee.SetFeature(Boid::eFleeRanged);
        m_simpleFlee.SetFlee(&m_simpleFollower);

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

        time += deltaTime;
        stopTimer -= deltaTime;

        if (stopTimer < 0.0f)
        {
            stopTimer = stopInterval;
            m_movingTarget = glm::normalize(glm::vec3(
                1.0f - sin(time) * 2.0f,
                1.0f - cos(time) * 2.0f,
                0.0f
            )) * 25.0f;
        }

        m_constantMovingTarget = glm::normalize(glm::vec3(
            sin(time * 0.5f),
            cos(time * 0.5f),
            0.0f
        )) * 4.0f;

        DebugDraw::AddAABB(
            m_movingTarget - glm::vec3(0.1f),
            m_movingTarget + glm::vec3(0.1f), { 0.7f, 0.3f, 0.4f, 1.0f });

        DebugDraw::AddAABB(
            m_constantMovingTarget - glm::vec3(0.1f),
            m_constantMovingTarget + glm::vec3(0.1f), { 0.7f, 0.3f, 0.4f, 1.0f });


        // DebugDraw::AddPosition(m_movingTarget, 0.5f, { 0.6f, 0.2f, 0.4f, 1.0f });
        // DebugDraw::AddPosition(m_constantMovingTarget, 0.5f, { 0.6f, 0.2f, 0.4f, 1.0f });

        m_viewGrid.Draw();
        DebugDraw::AddAABB(glm::vec3(0.0f) - glm::vec3(50.0f, 0.0f, 50.0f),
            glm::vec3(0.0f) + glm::vec3(50.0f, 0.0f, 50.0f));

        AABB limits = AABB(glm::vec3(0.0f, 25.0f, 0.0f), 50);
        DebugDraw::AddAABB(limits.GetMin(), limits.GetMax());

        std::vector<size_t> neighborIndices;
        {
#if USE_OCTREE
            m_octree = Octree(glm::vec3(0.0f), 50.0f);
            OcNode nodeData;
            for (size_t i = 0; i < ENTITY_COUNT; i++)
            {
                m_octree.Insert(m_wanderers[i].m_position, i);
            }
#endif

            for (size_t i = 0; i < ENTITY_COUNT; i++)
            {
#if USE_OCTREE
                AABB searchAabb = AABB(m_wanderers[i].m_position, m_wanderers[i].m_properties->m_neighborRange);
                std::vector<OcNode> neighborResult;
                m_octree.Search(searchAabb, neighborResult);
                neighborIndices.clear();
                for (const OcNode& node : neighborResult)
                {
                    if (node.m_storeIndex == i) 
                    {
                        continue;
                    }
                    neighborIndices.push_back(node.m_storeIndex);
                }
#endif
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

        {
            m_simpleFollower.SetTarget(m_constantMovingTarget);
            m_simpleFollower.FullUpdate(deltaTime, m_wanderers, neighborIndices);
            m_simpleFollower.DrawDebug();
        }

        {
            m_simpleArriver.SetTarget(m_movingTarget);
            m_simpleArriver.FullUpdate(deltaTime, m_wanderers, neighborIndices);
            m_simpleArriver.DrawDebug();
        }

        {
            m_simpleFlee.FullUpdate(deltaTime, m_wanderers, neighborIndices);
            m_simpleFlee.DrawDebug();
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

    // render representation
    Sphere* m_sphere = nullptr;

    Boid m_simpleFollower;
    Boid m_simpleArriver;
    Boid m_simpleFlee;
    Boid m_simplePathFollower;
    Boid m_simplePathFollower2;
    glm::vec3 m_movingTarget = { 1.0f, 1.0f, 1.0f };
    glm::vec3 m_constantMovingTarget = { 1.0f, 1.0f, 1.0f };

    float time = {};
    float stopTimer = {};
    float stopInterval = 5.0f;

    std::vector<glm::vec3> m_randomPositions;

    Boid::Properties m_sharedBoidProperties;
    std::vector<Boid> m_wanderers;
    Path m_path;
    Path m_path2;

    Octree m_octree;
};
