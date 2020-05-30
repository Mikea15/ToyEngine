#include "BaseState.h"
#include <Game.h>

void BaseState::Init(Game* game)
{
    
    m_game = game;
    m_renderer = m_game->GetRenderer();
    m_renderer->SetCamera(&m_camera);
}

void BaseState::HandleInput(SDL_Event* event)
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
}

void BaseState::Update(float deltaTime)
{
    if (m_inputGrabMouse) {
        int x, y;
        SDL_GetRelativeMouseState(&x, &y);
        m_camera.HandleMouse(static_cast<float>(x), static_cast<float>(-y));
    }

    // get camera movement input
    glm::vec3 inputDir(m_inputMoveRight, m_inputMoveUp, m_inputMoveForward);
    m_camera.HandleMove(deltaTime, inputDir, m_inputEnableMovementBoost);
    m_camera.Update(deltaTime);
}

void BaseState::Render(float alpha /*= 1.0f*/)
{
    m_renderer->RenderPushedCommands();
}

void BaseState::RenderUI()
{

}

void BaseState::Cleanup()
{

}
