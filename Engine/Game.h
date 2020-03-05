#pragma once

#include <memory>
#include <chrono>

#include "Camera/FlyCamera.h"
#include "Renderer/SimpleRenderer.h"
#include "SystemComponents/SystemComponentManager.h"
#include "Systems/GameFrameTime.h"
#include "Window/SDLHandler.h"

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

	double GetTimeMS() const { return m_time; }

private:
	void InitSystems();
	void CleanupSystems();

	void LoadConfig();
	void SaveConfig();

private:
	bool m_isRunning;

	GameFrameTime m_frameTime;

	float m_deltaTime = 0.0f;
	double m_time = 0.0f;

	SimpleRenderer* m_renderer{};
	// Renderer* m_renderer{};
	State* m_gameState{};
	SDLHandler m_sdlHandler;

	SystemComponentManager* m_systemComponents{};
};

