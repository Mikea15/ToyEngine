#pragma once

#include "Engine/State.h"

#include "OOP/Boid.h"
#include "Path.h"

#include "Engine/Core/AABBOctree.h"
#include "Engine/Core/Octree.h"
#include "Engine/Systems/KDTree.h"
#include "Engine/SystemComponents/StatSystemComponent.h"

class Game;

#define NEW_OCTREE 0

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

        for (size_t i = 0; i < 30; i++)
        {
            randomPoints.push_back(
                MathUtils::RandomInUnitSphere() * 10.0f
            );
        }

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

        for (size_t i = 0; i < ENTITY_COUNT; i++)
        {
#if USE_OCTREE
            QueryOctree(m_wanderers[i].m_position, m_wanderers[i].m_properties->m_neighborRange, i);
#elif USE_KDTREE
            QueryKDTree(m_wanderers[i].m_position, m_wanderers[i].m_properties->m_neighborRange, i);
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
#elif USE_KDTREE
        m_kdtree.DebugDraw();
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
#if !NEW_OCTREE
        m_octree = AABBOctree(glm::vec3(0.0f), 50.0f);
        for (size_t i = 0; i < ENTITY_COUNT; i++)
        {
            m_octree.Insert(m_wanderers[i].m_position, i);
        }
#else
        std::vector<glm::vec3> points(ENTITY_COUNT);
        for (size_t i = 0; i < ENTITY_COUNT; i++) 
        {
            points[i] = m_wanderers[i].m_position;
        }
        m_coreOctree.Initialize(points);
#endif
    }

    void QueryOctree(glm::vec3 pos, float range, size_t agentIndex)
    {
#if !NEW_OCTREE
        AABB searchAabb = AABB(pos, range);

        neighborResult.clear();
        // m_octree.FindNeighbors(pos, range, neighborResult);
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
#else
        m_coreOctree.FindNeighbors(pos, range, neighborIndices);
#endif
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

private:
    ViewportGrid m_viewGrid;

    Boid m_simplePathFollower;
    Boid m_simplePathFollower2;

    Properties m_sharedBoidProperties;
    std::vector<Boid> m_wanderers;

    Path m_path;
    Path m_path2;

    core::Octree m_coreOctree;

    AABBOctree m_octree;
    std::vector<OcNode> neighborResult;
    std::vector<size_t> neighborIndices;

    kdtree m_kdtree;
    std::vector<glm::vec3> randomPoints;
};
