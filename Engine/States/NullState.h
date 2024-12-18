#pragma once

#include "IGameState.h"

class NullState
    : public IGameState
{
public:
    ~NullState() override {};

    void Init(Game* game) override {};
    void HandleInput(SDL_Event* event) override {};
    void Update(float deltaTime) override {};
    void UpdatePaused(float deltaTime) override {};
    void Render(float alpha = 1.0f) override {};
    void RenderUI() override {};
    void Cleanup() override {};
};
