#include "StatSystemComponent.h"

#include "Game.h"

CLASS_DEFINITION(SystemComponent, StatSystemComponent)

StatSystemComponent::StatSystemComponent()
{
}

StatSystemComponent::~StatSystemComponent()
{
}

void StatSystemComponent::Initialize(Game* game)
{

}

void StatSystemComponent::HandleInput(SDL_Event* event)
{

}

void StatSystemComponent::PreUpdate(float frameTime)
{
	float frameTimeMS = frameTime * 1000.0f;
	
	m_totalTime += frameTime;
	m_oneSecond += frameTime;

	if (m_oneSecond >= 1.0f)
	{
		m_oneSecond -= 1.0f;
		m_updatesPerSecond = m_updateCount;
		m_framesPerSecond = m_renderCount;

		m_updateCount = 0;
		m_renderCount = 0;

		m_minFrameTime = FLT_MAX;
		m_maxFrameTime = -FLT_MAX;

		m_minFps = FLT_MAX;
		m_maxFps = -FLT_MAX;
	}

	m_lastFrameTimeMS = frameTimeMS;
}

void StatSystemComponent::Update(float deltaTime)
{
	m_lastDeltaTime = deltaTime;
	++m_updateCount;

	m_msBuffer[m_currentMsBufferIndex] = m_lastFrameTimeMS;
	if (m_currentMsBufferIndex < s_plotBufferSize - 1)
	{
		m_currentMsBufferIndex++;
	}
	else
	{
		for (int i = 0; i < s_plotBufferSize - 1; ++i)
		{
			m_msBuffer[i] = m_msBuffer[i + 1];

			m_minFrameTime = (m_msBuffer[i] < m_minFrameTime) ? m_msBuffer[i] : m_minFrameTime;
			m_maxFrameTime = (m_msBuffer[i] > m_maxFrameTime) ? m_msBuffer[i] : m_maxFrameTime;
		}
	}

	m_fpsBuffer[m_currentFpsBufferIndex] = m_framesPerSecond;
	if (m_currentFpsBufferIndex < s_plotBufferSize - 1)
	{
		m_currentFpsBufferIndex++;
	}
	else
	{
		for (int i = 0; i < s_plotBufferSize - 1; ++i)
		{
			m_fpsBuffer[i] = m_fpsBuffer[i + 1];

			m_minFps = (m_fpsBuffer[i] < m_minFps) ? m_fpsBuffer[i] : m_minFps;
			m_maxFps = (m_fpsBuffer[i] > m_maxFps) ? m_fpsBuffer[i] : m_maxFps;
		}
	}
}

void StatSystemComponent::Render(float alpha)
{
	++m_renderCount;
}

void StatSystemComponent::RenderUI()
{
	if (show_demo_window)
		ImGui::ShowDemoWindow(&show_demo_window);

	ImGui::Begin("Stats");

	ImGui::Columns(2, NULL, false);

	const int padding = 1;

	// frame time
	std::stringstream fmt;
	fmt << m_lastFrameTimeMS << " (ms)";
	ImGui::PushItemWidth(-padding);
	ImGui::PlotLines("", m_msBuffer, IM_ARRAYSIZE(m_msBuffer), 0,
		fmt.str().c_str(), m_maxFrameTime, m_minFrameTime, ImVec2(0, 50));
	ImGui::NextColumn();

	ImGui::PushItemWidth(15);
	ImGui::Text("Total Time: %0.2f", m_totalTime);
	ImGui::Text("Frame Time: %0.1f", m_lastFrameTimeMS);
	ImGui::Text("[ Min: %.1f / Max: %.1f ]", m_minFrameTime, m_maxFrameTime);
	ImGui::NextColumn();

	// fps
	std::stringstream fmtFps;
	fmtFps << "FPS: " << m_framesPerSecond;
	ImGui::PushItemWidth(-padding);
	ImGui::PlotLines("", m_fpsBuffer, IM_ARRAYSIZE(m_fpsBuffer), 0,
		fmtFps.str().c_str(), m_minFps, m_maxFps, ImVec2(0, 50));
	ImGui::NextColumn();

	ImGui::PushItemWidth(15);
	ImVec4 color = (m_framesPerSecond < 30) ? ImVec4(0.9f, 0.7f, 0.f, 1.f) : ImVec4(0.1f, 0.75f, 0.1f, 1.0f);
	ImGui::TextColored(color, fmtFps.str().c_str());
	ImGui::Text("[ Min %.1f / Max: %.1f ]", m_minFps, m_maxFps);
	ImGui::NextColumn();

	ImGui::Separator();

	ImGui::Text("DeltaTime: %.2f (ms)", m_lastDeltaTime);
	ImGui::Text("Updates p/ Second: %.2f", m_updatesPerSecond);
	ImGui::Text("1 (s): %.1f", m_oneSecond);

	ImGui::NextColumn();

	ImGui::Text("Update Count: %.0f", m_updateCount);
	ImGui::Text("Frame Count: %.0f", m_renderCount);
	ImGui::Text("Updates / Frame: %.2f", m_updateCount / m_renderCount);
	ImGui::NextColumn();

	ImGui::Columns(1);
	ImGui::Separator();

	ImGui::Checkbox("Demo window", &show_demo_window);
	ImGui::End();

	// 3. Show another simple window.
	if (show_another_window)
	{
		ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
		ImGui::Text("Hello from another window!");
		if (ImGui::Button("Close Me"))
			show_another_window = false;
		ImGui::End();
	}
}

void StatSystemComponent::Cleanup()
{

}
