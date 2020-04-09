
#include "Game.h"

#include <algorithm>
#include <cmath>

#include "State.h"
#include "Resources/Resources.h"
#include "Scene/Scene.h"

#include "SystemComponents/StatSystemComponent.h"

#include "Utils/Logger.h"

Game::Game(State* state)
	: m_isRunning(true)
	, m_deltaTime(1.0f / 60.0f)
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
	m_systemComponents->AddComponent<StatSystemComponent>();

	m_systemComponents->Initialize(this);

	Resources::Init();

	// Physx
	m_physxHandler.Init();

	m_renderer = new SimpleRenderer();
	m_renderer->Init();
	m_renderer->SetRenderSize(m_sdlHandler.GetWindowParams().Width, m_sdlHandler.GetWindowParams().Height);

}

void Game::CleanupSystems()
{
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


void Game::SetState(State* state)
{
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
	m_frameTime.Init();
	float accumulator = 0.0f;

	while (m_isRunning)
	{
		float frameTime = m_frameTime.GetFrameTime();
		m_time += frameTime;

		// cap on how many updates we can do.
		// eg. dt: 1/60 ( 60 ups ).
		// frametime: 0.25
		// max ups = frametime / m_deltaTime
		// max ups = 0.25 / 0.016 = 15.6ups
		// for 30 ups -> max frametime 0.25 ( 7.5 )
		// for 60 ups -> max frametime 0.083 ( 5 updates )
		if (frameTime > 0.25f)
			frameTime = 0.25f;

		accumulator += frameTime;

		// System Components PreUpdate
		m_systemComponents->PreUpdate(frameTime);

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

		// Handle Updates / Fixed Update
		while (accumulator >= m_deltaTime)
		{
			// physx tick
			// m_physxHandler.Tick(m_deltaTime);

			// system components
			m_systemComponents->Update(m_deltaTime);

			// window Update
			m_sdlHandler.Update(m_deltaTime);

			// Update
			m_gameState->Update(m_deltaTime);

			accumulator -= m_deltaTime;
		}


		// Handle Rendering
		const float alpha = accumulator / m_deltaTime;

		// Render
		m_sdlHandler.BeginRender();
		m_systemComponents->Render(alpha);
		m_gameState->Render(alpha);

		// ui
		m_sdlHandler.BeginUIRender();
		m_systemComponents->RenderUI();
		m_gameState->RenderUI();
		m_sdlHandler.EndUIRender();

		m_sdlHandler.EndRender();
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
