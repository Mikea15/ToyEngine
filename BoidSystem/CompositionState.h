#pragma once

#include "Engine/State.h"

#include "Definitions.h"

#include "Agent.h"
#include "Behaviours/IBehavior.h"

#include "OOP/Boid.h"
#include "Path.h"

class CompositionState
    : public BaseState
{
public:
    CompositionState()
        : BaseState()
    {}

    ~CompositionState() override {};

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

        m_simplePathFollower = Agent::CreateAgent();
        m_simplePathFollower.m_features = eFollowPath;
        m_simplePathFollower.m_properties = &m_sharedBoidProperties;
        m_simplePathFollower.m_path = &m_path;

        m_simplePathFollower2 = Agent::CreateAgent();
        m_simplePathFollower2.m_features = eFollowPath;
        m_simplePathFollower2.m_properties = &m_sharedBoidProperties;
        m_simplePathFollower2.m_path = &m_path2;

        for (size_t i = 0; i < ENTITY_COUNT; i++)
        {
            auto a = Agent::CreateAgent();
            a.m_properties = &m_sharedBoidProperties;
            a.m_features =
                eSeek |
                eFlee |
                eAlignment |
                eSeparation |
                eCohesion |
                eWallLimits;

            if (MathUtils::Rand01() > 0.5f)
            {
                a.target = &m_simplePathFollower;
                a.flee = &m_simplePathFollower2;
            }
            else
            {
                a.target = &m_simplePathFollower2;
                a.flee = &m_simplePathFollower;
            }

            a.m_position = glm::vec3(
                MathUtils::Rand(-50.0f, 50.0f),
                MathUtils::Rand(-50.0f, 50.0f),
                MathUtils::Rand(-50.0f, 50.0f)
            );

            m_wanderers.push_back(a);
        }

        m_world.neighbors = &m_wanderers;
        m_world.neighborIndices.resize(ENTITY_COUNT);
        m_world.limits = AABB(glm::vec3(0.0f, 25.0f, 0.0f), 50);

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
            //m_octree = Octree(glm::vec3(0.0f), 50.0f);
            //OcNode nodeData;
            //for (size_t i = 0; i < ENTITY_COUNT; i++)
            //{
            //    m_octree.Insert(m_wanderers[i].m_position, i);
            //}
#endif

            for (size_t i = 0; i < ENTITY_COUNT; i++)
            {
#if USE_OCTREE
                //AABB searchAabb = AABB(m_wanderers[i].m_position, m_wanderers[i].m_properties->m_neighborRange);
                //neighborResult.clear();

                //m_octree.Search(searchAabb, neighborResult);
                //neighborIndices.clear();
                //for (const OcNode& node : neighborResult)
                //{
                //    if (node.m_storeIndex == i)
                //    {
                //        continue;
                //    }
                //    neighborIndices.insert(node.m_storeIndex);
                //}
#endif // USE_OCTREE
                // Agent Core Loop
                {
                    Agent* agent = &m_wanderers[i];

                    m_world.agent = agent;
                    Search(agent, m_world);

                    const glm::vec3 force = m_steeringBehavior.CalculateWeighted(&m_world);
                    const glm::vec3 acceleration = force / agent->m_properties->m_mass;

                    agent->m_velocity += acceleration * deltaTime;
                    agent->m_velocity = glm::clamp(agent->m_velocity,
                        -agent->m_properties->m_maxSpeed,
                        agent->m_properties->m_maxSpeed);

                    if (glm::length(agent->m_velocity) > 0.0001f)
                    {
                        agent->m_heading = glm::normalize(agent->m_velocity);
                    }
                    agent->m_position += agent->m_velocity * deltaTime;
                
                    DebugDraw::AddLine(agent->m_position, agent->m_position + agent->m_velocity, { 0.75f, 0.0f, 1.0f, 1.0f });
                    DebugDraw::AddLine(agent->m_position, agent->m_position + agent->m_heading, { 0.0f, 0.75f, 1.0f, 1.0f });
                }
            }
#if USE_OCTREE
            // m_octree.DebugDraw();
#endif
        }

        {
            m_path.DebugDraw();

            m_world.agent = &m_simplePathFollower;

            const glm::vec3 force = m_steeringBehavior.CalculateWeighted(&m_world);
            const glm::vec3 acceleration = force / m_simplePathFollower.m_properties->m_mass;

            m_simplePathFollower.m_velocity += acceleration * deltaTime;
            m_simplePathFollower.m_velocity = glm::clamp(m_simplePathFollower.m_velocity,
                -m_simplePathFollower.m_properties->m_maxSpeed,
                m_simplePathFollower.m_properties->m_maxSpeed);

            if (glm::length(m_simplePathFollower.m_velocity) > 0.0001f)
            {
                m_simplePathFollower.m_heading = glm::normalize(m_simplePathFollower.m_velocity);
            }
            m_simplePathFollower.m_position += m_simplePathFollower.m_velocity * deltaTime;

            DebugDraw::AddLine(m_simplePathFollower.m_position, m_simplePathFollower.m_position + m_simplePathFollower.m_velocity, { 0.75f, 0.0f, 1.0f, 1.0f });
            DebugDraw::AddLine(m_simplePathFollower.m_position, m_simplePathFollower.m_position + m_simplePathFollower.m_heading, { 0.0f, 0.75f, 1.0f, 1.0f });
        }

        {
            m_path2.DebugDraw();

            m_world.agent = &m_simplePathFollower2;

            const glm::vec3 force = m_steeringBehavior.CalculateWeighted(&m_world);
            const glm::vec3 acceleration = force / m_simplePathFollower2.m_properties->m_mass;

            m_simplePathFollower2.m_velocity += acceleration * deltaTime;
            m_simplePathFollower2.m_velocity = glm::clamp(m_simplePathFollower2.m_velocity,
                -m_simplePathFollower2.m_properties->m_maxSpeed,
                m_simplePathFollower2.m_properties->m_maxSpeed);

            if (glm::length(m_simplePathFollower2.m_velocity) > 0.0001f)
            {
                m_simplePathFollower2.m_heading = glm::normalize(m_simplePathFollower2.m_velocity);
            }
            m_simplePathFollower2.m_position += m_simplePathFollower2.m_velocity * deltaTime;

            DebugDraw::AddLine(m_simplePathFollower2.m_position, m_simplePathFollower2.m_position + m_simplePathFollower2.m_velocity, { 0.75f, 0.0f, 1.0f, 1.0f });
            DebugDraw::AddLine(m_simplePathFollower2.m_position, m_simplePathFollower2.m_position + m_simplePathFollower2.m_heading, { 0.0f, 0.75f, 1.0f, 1.0f });
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

    AgentWorld m_world;
    Agent m_simplePathFollower;
    Agent m_simplePathFollower2;

    Properties m_sharedBoidProperties;
    std::vector<Agent> m_wanderers;

    SteeringBehavior m_steeringBehavior;

    Path m_path;
    Path m_path2;

};
