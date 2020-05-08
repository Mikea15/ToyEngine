#pragma once

#include <memory>
#include <chrono>

#include "Camera/FlyCamera.h"
#include "Renderer/SimpleRenderer.h"
#include "SystemComponents/SystemComponentManager.h"
#include "Systems/GameTime.h"
#include "Window/SDLHandler.h"
#include "PhysxHandler.h"

class State;
class WindowParams;

class Game
{
public:
	Game(State* state);
	~Game();

	void SetState(State* state);
	int Execute();

	SDLHandler* GetSDLHandler() { return &m_sdlHandler; }
	SimpleRenderer* GetRenderer() { return m_renderer; }
	PhysXHandler* GetPhysX() { return &m_physxHandler; }
	SystemComponentManager* GetSystemComponentManager() { return m_systemComponents; }

	TimePrecision GetTotalTime() const { return m_gameTime.GetTotalTime(); }

private:
	void InitSystems();
	void CleanupSystems();

	void LoadConfig();
	void SaveConfig();

private:
	bool m_isRunning;

	GameTime m_gameTime;

	SimpleRenderer* m_renderer{};
	// Renderer* m_renderer{};
	State* m_gameState{};
	SDLHandler m_sdlHandler;
	PhysXHandler m_physxHandler;

	SystemComponentManager* m_systemComponents{};
};

