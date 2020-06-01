#pragma once

#include "IGameState.h"
#include "Camera/FlyCamera.h"

class SimpleRenderer;


class BaseState
    : public IGameState
{
public:
    ~BaseState() override {}

    void Init(Game* game) override;
    void HandleInput(SDL_Event* event) override;
    void Update(float deltaTime) override;
    void UpdatePaused(float deltaTime) override;
    void Render(float alpha = 1.0f) override;
    void RenderUI() override;
    void Cleanup() override;

    void HandleSceneCamera(float deltaTime);

protected:
    Game* m_game = nullptr;
    SimpleRenderer* m_renderer = nullptr;
    FlyCamera m_camera = FlyCamera(glm::vec3(0.0f, 5.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));

    bool m_inputGrabMouse = false;
    float m_inputMoveUp = 0.0f;
    float m_inputMoveRight = 0.0f;
    float m_inputMoveForward = 0.0f;
    bool m_inputEnableMovementBoost = false;
};