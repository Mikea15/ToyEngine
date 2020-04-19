#pragma once

#include "SystemComponentManager.h"

class StatSystemComponent
	: public SystemComponent
{
	CLASS_DECLARATION(StatSystemComponent)

public:
	StatSystemComponent();
	~StatSystemComponent() override;

	void Initialize(Game* game) override;
	void HandleInput(SDL_Event* event) override;
	void PreUpdate(float frameTime) override;
	void Update(float deltaTime) override;
	void Render(float alpha) override;
	void RenderUI() override;
	void Cleanup() override;

	float GetAverageFPS() const;
	float GetAverageMS() const;

private:
	float m_totalTime = 0.0f;
	float m_renderCount = 0.0f;
	float m_updateCount = 0.0f;
	float m_oneSecond = 0.0f;
	float m_updatesPerSecond = 0.0f;
	float m_framesPerSecond = 0.0f;

	float m_minFps = 9999.0f;
	float m_maxFps = 0.0f;

	float m_minFrameTime = 9999.0f;
	float m_maxFrameTime = 0.0f;

	float m_lastFrameTimeMS = 0.0f;
	float m_lastDeltaTime = 0.0f;

	static const int s_sampleCount = 350;
	
	float m_sampleInterval = 0.1f; //	100 sample count * 0.1 sample interval = 10 seconds of samples.
	float m_currentSampleInterval = 0.0f;

	int m_currentMsBufferIndex = 0;
	float m_msBuffer[s_sampleCount] = { -1.0f };
	int m_currentFpsBufferIndex = 0;
	float m_fpsBuffer[s_sampleCount] = { -1.0f };

	bool show_demo_window = false;
	bool show_another_window = false;
	bool open = true;
};