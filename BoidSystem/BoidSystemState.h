#pragma once

#include "Engine/State.h"

#include "Boids/Boid.h"

class Game;

#if _DEBUG
#define ENTITY_COUNT 1500
#else
#define ENTITY_COUNT 1500
#endif

struct Path
{
    Path() = default;
    Path(std::vector<glm::vec3> path)
    {
        m_nodes = path;
        m_currentNodeIndex = static_cast<unsigned int>(MathUtils::Rand(0, m_nodes.size() - 1));
    }

    glm::vec3 GetCurrentGoal() const { return m_nodes[m_currentNodeIndex]; }
    void UpdatePath( glm::vec3 position )
    {
        float distanceToGoal = glm::length(GetCurrentGoal() - position);
        if (distanceToGoal <= atGoalDistanceThreshold)
        {
            m_currentNodeIndex++;
            if (m_currentNodeIndex > m_nodes.size() - 1) 
            {
                m_currentNodeIndex = 0;
            }
        }
    }

    void DebugDraw()
    {
        if (m_nodes.empty())
        {
            return;
        }

        for (size_t i = 0; i < m_nodes.size(); i++)
        {
            if (i == m_currentNodeIndex)
            {
                DebugDraw::AddAABB(
                    m_nodes[i] - glm::vec3(atGoalDistanceThreshold), 
                    m_nodes[i] + glm::vec3(atGoalDistanceThreshold),
                    { 0.3f, 0.6f, 0.1, 1.0f });
            }

            if (i != m_nodes.size() - 1) 
            {
                DebugDraw::AddLine(m_nodes[i], m_nodes[i + 1], { 0.3f, 0.6f, 0.1, 1.0f });
            }
        }
        DebugDraw::AddLine(m_nodes[m_nodes.size() - 1], m_nodes[0], { 0.3f, 0.6f, 0.1, 1.0f });
    }

    size_t m_currentNodeIndex;
    std::vector<glm::vec3> m_nodes;

    float atGoalDistanceThreshold = 2.0f;
};

class BoidSystemState
    : public State
{
public:
    BoidSystemState()
        : State()
    {}

    ~BoidSystemState() override {};

    void Init(Game* game) override
    {
        gamePtr = game;
        m_renderer = gamePtr->GetRenderer();

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

        for (size_t i = 0; i < ENTITY_COUNT; i++)
        {
            auto b = Boid(&m_boidProperties);
            auto features = 
                Boid::eSeek | 
                //Boid::eAlignment | 
                Boid::eSeparation | 
                //Boid::eCohesion | 
                Boid::eWallLimits;

            b.SetFeature(features);

            // b.m_maxAccelerationForce = MathUtils::Rand01() * 0.1f;
            // b.m_maxVelocity = MathUtils::Rand01() * 5.0f;
            b.m_position = glm::vec3(
                MathUtils::Rand(-50.0f, 50.0f),
                MathUtils::Rand(-50.0f, 50.0f),
                MathUtils::Rand(-50.0f, 50.0f)
            );

            b.SetTarget(&m_simplePathFollower);

            m_wanderers.push_back(b);
        }

        m_simpleFollower.SetFeature(Boid::eSeek);
        m_simplePathFollower.SetFeature(Boid::eSeek);

        m_simpleArriver.SetFeature(Boid::eArrive);

        m_simpleFlee.SetFeature(Boid::eFleeRanged);
        m_simpleFlee.SetFlee(&m_simpleFollower);

        DebugDraw::Init();
    };

    void HandleInput(SDL_Event* event) override
    {
        if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_F1)
        {
            m_inputGrabMouse = !m_inputGrabMouse;
            SDL_SetRelativeMouseMode(m_inputGrabMouse ? SDL_TRUE : SDL_FALSE);
        }

        if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_w) m_inputMoveForward = 1;
        if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_s) m_inputMoveForward = -1;
        if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_a) m_inputMoveRight = -1;
        if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_d) m_inputMoveRight = 1;
        if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_e) m_inputMoveUp = 1;
        if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_q) m_inputMoveUp = -1;
        if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_LSHIFT) m_inputEnableMovementBoost = true;

        if (event->type == SDL_KEYUP && event->key.keysym.sym == SDLK_w) m_inputMoveForward = 0;
        if (event->type == SDL_KEYUP && event->key.keysym.sym == SDLK_s) m_inputMoveForward = 0;
        if (event->type == SDL_KEYUP && event->key.keysym.sym == SDLK_a) m_inputMoveRight = 0;
        if (event->type == SDL_KEYUP && event->key.keysym.sym == SDLK_d) m_inputMoveRight = 0;
        if (event->type == SDL_KEYUP && event->key.keysym.sym == SDLK_e) m_inputMoveUp = 0;
        if (event->type == SDL_KEYUP && event->key.keysym.sym == SDLK_q) m_inputMoveUp = 0;
        if (event->type == SDL_KEYUP && event->key.keysym.sym == SDLK_LSHIFT) m_inputEnableMovementBoost = false;
    };

    void Update(float deltaTime) override
    {
        glm::vec3 mouseWorldPos = {};
        if (m_inputGrabMouse) {
            int x, y;
            SDL_GetRelativeMouseState(&x, &y);
            m_camera.HandleMouse(static_cast<float>(x), static_cast<float>(-y));

            mouseWorldPos = m_camera.ScreenSpaceToWorldSpace(
                glm::vec3(static_cast<float>(x), static_cast<float>(-y), 0.f)
            );
        }

        // get camera movement input
        glm::vec3 inputDir(m_inputMoveRight, m_inputMoveUp, m_inputMoveForward);
        m_camera.HandleMove(deltaTime, inputDir, m_inputEnableMovementBoost);

        m_camera.Update(deltaTime);

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

        for (size_t i = 0; i < ENTITY_COUNT; i++)
        {
            m_wanderers[i].Update(deltaTime, m_wanderers);
            m_wanderers[i].DrawDebug();
        }

        {
            m_path.UpdatePath(m_simplePathFollower.m_position);
            m_path.DebugDraw();

            m_simplePathFollower.SetTarget(m_path.GetCurrentGoal());
            m_simplePathFollower.Update(deltaTime, m_wanderers);
            m_simplePathFollower.DrawDebug();
        }

        {
            m_simpleFollower.SetTarget(m_constantMovingTarget);
            m_simpleFollower.Update(deltaTime, m_wanderers);
            m_simpleFollower.DrawDebug();
        }

        {
            m_simpleArriver.SetTarget(m_movingTarget);
            m_simpleArriver.Update(deltaTime, m_wanderers);
            m_simpleArriver.DrawDebug();
        }

        {
            m_simpleFlee.Update(deltaTime, m_wanderers);
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
        Debug::ShowPanel(m_boidProperties);
    };

    void Cleanup() override {};

private:
    Game* gamePtr;
    SimpleRenderer* m_renderer;
    FlyCamera m_camera;
    ViewportGrid m_viewGrid;
    // render representation
    Sphere* m_sphere;

    bool m_inputGrabMouse = false;
    float m_inputMoveUp = 0.0f;
    float m_inputMoveRight = 0.0f;
    float m_inputMoveForward = 0.0f;
    bool m_inputEnableMovementBoost = false;

    Boid m_simpleFollower;
    Boid m_simpleArriver;
    Boid m_simpleFlee;
    Boid m_simplePathFollower;
    glm::vec3 m_movingTarget = { 1.0f, 1.0f, 1.0f };
    glm::vec3 m_constantMovingTarget = { 1.0f, 1.0f, 1.0f };

    float time = {};
    float stopTimer = {};
    float stopInterval = 5.0f;

    std::vector<glm::vec3> m_randomPositions;

    Boid::Properties m_boidProperties;
    std::vector<Boid> m_wanderers;
    Path m_path;
};
