#include "StatSystemComponent.h"

#include "Game.h"

#include "Utils/FileIO.h"

CLASS_DEFINITION(SystemComponent, StatSystemComponent)

StatSystemComponent::StatSystemComponent()
{
}

StatSystemComponent::~StatSystemComponent()
{
	std::stringstream stream;
	WriteInfo(stream);
	FileIO::SaveTextFile("debug_info.txt", stream.str(), std::fstream::out | std::fstream::app);
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

		for (int i = 0; i < s_sampleCount - 1; ++i)
		{
			m_minFrameTime = (m_msBuffer[i] < m_minFrameTime) ? m_msBuffer[i] : m_minFrameTime;
			m_maxFrameTime = (m_msBuffer[i] > m_maxFrameTime) ? m_msBuffer[i] : m_maxFrameTime;

			m_minFps = (m_fpsBuffer[i] < m_minFps) ? m_fpsBuffer[i] : m_minFps;
			m_maxFps = (m_fpsBuffer[i] > m_maxFps) ? m_fpsBuffer[i] : m_maxFps;
		}
	}

	m_lastFrameTimeMS = frameTimeMS;
}

void StatSystemComponent::Update(float deltaTime)
{
	m_lastDeltaTime = deltaTime;
	++m_updateCount;

	m_currentSampleInterval += deltaTime;
	if (m_currentSampleInterval < m_sampleInterval)
	{
		return;
	}
	m_currentSampleInterval = 0.0f;

	m_msBuffer[m_currentMsBufferIndex++] = m_lastFrameTimeMS;
	if (m_currentMsBufferIndex >= s_sampleCount)
	{
		m_currentMsBufferIndex = 0;
	}

	m_fpsBuffer[m_currentFpsBufferIndex++] = m_framesPerSecond;
	if (m_currentFpsBufferIndex >= s_sampleCount)
	{
		m_currentFpsBufferIndex = 0;
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

	const float avgFPS = GetAverageFPS();
	const float avgMS = GetAverageMS();

	const int padding = 1;

	const float DISTANCE = 10.0f;
	static int corner = 1;
	ImGuiIO& io = ImGui::GetIO();
	if (corner != -1)
	{
		ImVec2 window_pos = ImVec2((corner & 1) ? io.DisplaySize.x - DISTANCE : DISTANCE, (corner & 2) ? io.DisplaySize.y - DISTANCE : DISTANCE);
		ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
		ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
	}

	ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
	ImGui::SetNextWindowSize(ImVec2(250, 120));

	static bool open = true;
	ImGui::Begin("Stats", &open, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing );
	
	// frame time
	char bufferMS[40];
	sprintf_s(bufferMS, "%.2f (ms) / %.2f (avg)", m_lastFrameTimeMS, avgMS);
	ImGui::PushItemWidth(-padding);
	ImGui::PlotLines("", m_msBuffer, IM_ARRAYSIZE(m_msBuffer), m_currentMsBufferIndex, bufferMS, 60, -5, ImVec2(0, 50));
	// ImGui::NextColumn();

	// fps
	char bufferFPS[40];
	sprintf_s(bufferFPS, "%.2f (fps) / %.2f (avg)", m_framesPerSecond, avgFPS);
	ImGui::PushItemWidth(-padding);
	ImGui::PlotLines("", m_fpsBuffer, IM_ARRAYSIZE(m_fpsBuffer), m_currentFpsBufferIndex, bufferFPS, 0, m_maxFps + 10, ImVec2(0, 50));
	// ImGui::NextColumn();
	ImGui::End();

	ImGui::Begin("Stats::Detailed");

	ImGui::Columns(2, NULL, false);
	ImGui::PushItemWidth(15);
	ImGui::Text("Frame Time: %0.2f", m_lastFrameTimeMS);
	ImGui::Text("Min: %.1f", m_minFrameTime);
	ImGui::Text("Max: %.1f", m_maxFrameTime);
	ImGui::Text("Avg: %.2f", avgMS);
	ImGui::NextColumn();

	ImGui::PushItemWidth(15);
	ImVec4 color = (m_framesPerSecond < 30) ? ImVec4(0.9f, 0.7f, 0.f, 1.f) : ImVec4(0.1f, 0.75f, 0.1f, 1.0f);
	ImGui::TextColored(color, "FPS: %.2f", m_framesPerSecond);
	ImGui::Text("Min %.1f", m_minFps);
	ImGui::Text("Max: %.1f", m_maxFps);
	ImGui::Text("Avg: %.2f", avgFPS);
	ImGui::NextColumn();

	ImGui::Separator();

	ImGui::Text("Total Time: %0.2f", m_totalTime);
	ImGui::Text("dt: %.3f (ms)", m_lastDeltaTime);
	ImGui::Text("Updates (p/sec): %.2f", m_updatesPerSecond);
	ImGui::Text("One (s): %.3f", m_oneSecond);
	ImGui::NextColumn();

	ImGui::Text("# Updates: %.1f", m_updateCount);
	ImGui::Text("# Frames: %.1f", m_renderCount);
	ImGui::Text("# Updates (p/frame): %.3f", m_updateCount / m_renderCount);
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

float StatSystemComponent::GetAverageFPS() const
{
	float avg = 0.0f;
	for (int i = 0; i < s_sampleCount - 1; ++i)
	{
		avg += m_fpsBuffer[i];
	}

	if (s_sampleCount > 0)
	{
		return avg / s_sampleCount;
	}
	return 0.0f;
}

float StatSystemComponent::GetAverageMS() const
{
	float avg = 0.0f;
	for (int i = 0; i < s_sampleCount - 1; ++i)
	{
		avg += m_msBuffer[i];
	}

	if (s_sampleCount > 0)
	{
		return avg / s_sampleCount;
	}
	return 0.0f;
}

void StatSystemComponent::WriteInfo(std::stringstream& stringStream)
{
	char buffer[150];
	sprintf_s(buffer, "Time: %0.1f - FPS Avg: %0.1f [Min: %0.1f, Max: %0.1f] - MS Avg: %0.1f [Min: %0.1f, Max: %0.1f]\n",
		m_totalTime, GetAverageFPS(), m_minFps, m_maxFps, 
		GetAverageMS(), m_minFrameTime, m_maxFrameTime);

	stringStream << buffer;
}
