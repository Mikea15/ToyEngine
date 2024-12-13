#include "Game.h"

#include <algorithm>
#include <cmath>

#include "States/IGameState.h"
#include "Resources/Resources.h"
#include "Scene/Scene.h"

#include "SystemComponents/StatSystemComponent.h"

#include "Utils/Logger.h"
#include "Core/Profiler.h"
#include "Core/JobScheduler/JobScheduler.h"


Game::Game(IGameState* state)
	: m_isRunning(true)
	, m_gameState(nullptr)
{
	LoadConfig();

	InitSystems();
	SetState(state);
}

Game::~Game()
{
	SaveConfig();
}

void Game::InitSystems()
{
	srand(static_cast<unsigned int>(time(NULL)));

	m_sdlHandler.Init();
	m_sdlHandler.OnExitWindow([this]() { m_isRunning = false; });

	// System Components
	m_systemComponents = new SystemComponentManager();
	m_systemComponents->AddComponent<StatSystemComponent>(&m_gameTime);

	m_systemComponents->Initialize(this);

	Resources::Init();

	// Physx
	m_physxHandler.Init();

	m_renderer = new SimpleRenderer();
	m_renderer->Init();
	m_renderer->SetRenderSize(m_sdlHandler.GetWindowParams().Width, m_sdlHandler.GetWindowParams().Height);

	JobScheduler::GetInstance().Init();
}

void Game::CleanupSystems()
{
	JobScheduler::GetInstance().Cleanup();

	delete m_renderer;
	delete m_systemComponents;

	Scene::Clear();
	Resources::Clean();

	m_physxHandler.Cleanup();

	m_systemComponents->Cleanup();

	// state cleanup
	m_gameState->Cleanup();
	m_gameState = nullptr;
}


void Game::SetState(IGameState* state)
{
	assert(state != nullptr);

	if (m_gameState != nullptr)
	{
		m_gameState->Cleanup();
		m_gameState = nullptr;
	}

	m_gameState = state;
	m_gameState->Init(this);
}

int Game::Execute()
{
	m_gameTime.Init();
	float accumulator = 0.0f;

	while (m_isRunning)
	{
		PROFILE_SCOPE("UpdateLoop");
		m_gameTime.Tick();

		const float frameTime = m_gameTime.GetElapsed();
		const float updateRate = m_gameTime.GetUpdateRate();
		accumulator += frameTime;

		// System Components PreUpdate
		m_systemComponents->PreUpdate(frameTime);

		{
			PROFILE_SCOPE("HandleInput");
			// Handle Input
			SDL_Event event;
			while (SDL_PollEvent(&event))
			{
				// Handle Input
				m_sdlHandler.HandleEvents(&event);

				switch (event.type)
				{
				case SDL_KEYDOWN:
					switch (event.key.keysym.sym)
					{
						break;
					case SDLK_ESCAPE:
						m_isRunning = false;
						break;
					default: break;
					}
					break;
				default: break;
				}

				// System Components Handle Input
				m_systemComponents->HandleInput(&event);

				m_gameState->HandleInput(&event);
			}
		}

		{
			PROFILE_SCOPE("Update");

			// Handle Updates at Fixed Update Rate
			// if update rate is 60fps, and frame time is 30fps.
			// accumulator 33ms, update rate, 16ms.
			// 1 update -> accumulator 33-16=17.
			// 2 update -> accumulator 17-16=1
			// no update 3 because accumulator = 1 < 16 ( update Rate )
			// we'll update twice, before rendering a new frame.
			while (accumulator >= updateRate)
			{
                // physx tick
                // m_physxHandler.Tick(m_deltaTime);

                // window Update
                m_sdlHandler.Update(updateRate);

				if (m_gameTime.IsPaused()) 
				{
					// system components
					m_systemComponents->UpdatePaused(updateRate);

					// Update
					m_gameState->UpdatePaused(updateRate);
				}
				else
				{
                    // system components
                    m_systemComponents->Update(updateRate);

                    // Update
                    m_gameState->Update(updateRate);
				}

				accumulator -= updateRate;
			}
		}


		// Handle Rendering
		const float alpha = accumulator / updateRate;

		// Render
		{
			PROFILE_SCOPE("RenderLoop");
			m_sdlHandler.BeginRender();
			m_systemComponents->Render(alpha);
			m_gameState->Render(alpha);

			// ui
			{
				PROFILE_SCOPE("RenderUI");
				m_sdlHandler.BeginUIRender();
				m_systemComponents->RenderUI();
				m_gameState->RenderUI();
				m_sdlHandler.EndUIRender();
			}

			m_sdlHandler.EndRender();
		}
	}

	CleanupSystems();

	return 0;
}

void Game::LoadConfig()
{

}

void Game::SaveConfig()
{

}
