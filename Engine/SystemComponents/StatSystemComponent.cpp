#include "StatSystemComponent.h"

#include "Game.h"

#include "Systems/GameTime.h"
#include "Utils/FileIO.h"

CLASS_DEFINITION(SystemComponent, StatSystemComponent)

StatSystemComponent::StatSystemComponent(GameTime* pGameTime)
    : m_pGameTime(pGameTime)
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
    m_oneSecond += m_pGameTime->GetElapsed();

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

    m_msBuffer[m_currentMsBufferIndex++] = m_pGameTime->GetElapsed() * 1000.0f; // to Milliseconds.
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
    ImGui::Begin("Stats", &open, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing);

    // frame time
    char bufferMS[40];
    sprintf_s(bufferMS, "%.2f (ms) / %.2f (avg)", m_pGameTime->GetElapsed(), avgMS);
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

    if (!m_info.empty())
    {
        ImGui::Separator();
        ImGui::Text(m_info.c_str());
        ImGui::Separator();
    }

    ImGui::Text("Ms: %.3f, Avg: %.2f (Min: %.2f / Max: %.2f)", (m_pGameTime->GetElapsed() / 1000.0f), avgMS, m_minFrameTime, m_maxFrameTime);
    ImGui::Text("FPS: %.2f, Avg: %.2f (Min: %.2f / Max: %.2f)", m_framesPerSecond, avgFPS, m_minFps, m_maxFps);

    ImGui::Separator();

    ImGui::Text("Total Time: %0.1f", m_pGameTime->GetTotalTime());
    ImGui::Text("Update Rate: %.2f", m_pGameTime->GetUpdateRate());
    ImGui::Text("DeltaTime: %.3f (ms)", m_pGameTime->GetElapsed());

    ImGui::Separator();

    ImGui::Text("TimeScale: %.1f", m_pGameTime->GetTimeScale());
    float timeScale = m_pGameTime->GetTimeScale();
    ImGui::SliderFloat("TimeScale", &timeScale, m_pGameTime->GetTimeScaleMin(),
        m_pGameTime->GetTimeScaleMax());
    if (m_pGameTime->GetTimeScale() != timeScale) {
        m_pGameTime->SetTimeScale(timeScale);
    }

    ImGui::Separator();

    ImGui::Text("Update: %.0f", m_updateCount);
    ImGui::Text("Frame: %.0f", m_renderCount);
    ImGui::Text("Updates (p/sec): %.1f", m_updatesPerSecond);
    ImGui::Text("Updates (p/frame): %.1f", m_updateCount / m_renderCount);

    ImGui::Separator();

    ImGui::Text("One (s): %.3f", m_oneSecond);

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
    for (int i = 0; i < m_currentFpsBufferIndex; ++i)
    {
        avg += m_fpsBuffer[i];
    }

    if (m_currentFpsBufferIndex > 0)
    {
        return avg / m_currentFpsBufferIndex;
    }
    return 0.0f;
}

float StatSystemComponent::GetAverageMS() const
{
    float avg = 0.0f;
    for (int i = 0; i < m_currentMsBufferIndex; ++i)
    {
        avg += m_msBuffer[i];
    }

    if (m_currentMsBufferIndex > 0)
    {
        return avg / m_currentMsBufferIndex;
    }
    return 0.0f;
}

void StatSystemComponent::WriteInfo(std::stringstream& stringStream)
{
    char buffer[150];
    sprintf_s(buffer, "Time: %0.1f - FPS Avg: %0.1f [Min: %0.1f, Max: %0.1f] - MS Avg: %0.1f [Min: %0.1f, Max: %0.1f]\n",
        m_pGameTime->GetTotalTime(), GetAverageFPS(), m_minFps, m_maxFps,
        GetAverageMS(), m_minFrameTime, m_maxFrameTime);

    stringStream << buffer;
}

void StatSystemComponent::SetCustomInfoLog(const std::string& info)
{
    m_info = info;
}
