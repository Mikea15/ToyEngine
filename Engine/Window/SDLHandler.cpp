#include "SDLHandler.h"

#include <string>
#include <iostream>
#include <fstream>

#include "WindowParams.h"
#include "Utils/FileIO.h"
#include "Utils/Utils.h"
#include "Utils/Logger.h"

std::string SDLHandler::s_configFileName = "../Data/config.json";

SDLHandler::SDLHandler()
{
	LoadWindowParams();
}

SDLHandler::~SDLHandler()
{
	SaveWindowParams();

	m_uiHandler.Cleanup();

	SDL_GL_DeleteContext(m_mainGLContext);
	SDL_DestroyWindow(m_window);
	SDL_Quit();
}

bool SDLHandler::Init()
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
	{
		std::cerr << "SDL Init Error\n";
		return false;
	}

	FindDisplayModes();

	// set initial params. don't want to pickup best mode for dev.
	const int width = m_windowParams.Width;
	const int height = m_windowParams.Height;

	m_window = SDL_CreateWindow( m_windowParams.WindowName.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
		width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

	const bool initialSetup = true;
	SetWindowParameters(m_windowParams, initialSetup);
	
	m_mainGLContext = SDL_GL_CreateContext(m_window);

	glewInit();

	// have this by default
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	glDepthRange(0.0f, 1.0f);

	m_uiHandler = IMGUIHandler();
	m_uiHandler.Initialize(m_window, &m_mainGLContext, m_glslVersion);

	return true;
}

void SDLHandler::HandleEvents(SDL_Event* event)
{
	m_uiHandler.HandleInput(event);

	switch (event->type)
	{
	case SDL_QUIT:
		m_onExitWindowCallback();
		break;
	case SDL_WINDOWEVENT:
		switch (event->window.event)
		{
		case SDL_WINDOWEVENT_CLOSE:
			m_onExitWindowCallback();
			break;
		case SDL_WINDOWEVENT_RESIZED:
			m_windowParams.Width = static_cast<int>(event->window.data1);
			m_windowParams.Height = static_cast<int>(event->window.data2);
			SDL_SetWindowSize(m_window, m_windowParams.Width, m_windowParams.Height);
			break;
		case SDL_WINDOWEVENT_FOCUS_GAINED:
			break;
		case SDL_WINDOWEVENT_FOCUS_LOST:
			break;
		default: break;
		}
		break;
	default: break;
	}
}

void SDLHandler::Update(float deltaTime)
{

}

void SDLHandler::BeginRender()
{
	SDL_GL_MakeCurrent(m_window, m_mainGLContext);
}

void SDLHandler::EndRender()
{
	SDL_GL_SwapWindow(m_window);
}

void SDLHandler::BeginUIRender()
{
	m_uiHandler.StartRender(m_window);
}

void SDLHandler::EndUIRender()
{
	m_uiHandler.EndRender();
}

void SDLHandler::SetWindowParameters(const WindowParams& params, bool initialSetup)
{
	m_windowParams = params;

	if (!initialSetup)
	{
		const SDL_DisplayMode* dm = GetCurrentDisplayMode();
		if (dm != nullptr)
		{
			SDL_SetWindowDisplayMode(m_window, dm);
			SDL_SetWindowSize(m_window, dm->w, dm->h);
			m_windowParams.Width = dm->w;
			m_windowParams.Height = dm->h;
		}
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, m_windowParams.GL_Flags);
	// SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, m_params->GL_ProfileMask);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, m_windowParams.GL_MajorVersion);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, m_windowParams.GL_MinorVersion);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, m_windowParams.GL_DoubleBuffer);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, m_windowParams.GL_Accelerated);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, m_windowParams.GL_DepthSize);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, m_windowParams.GL_StencilSize);

	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, m_windowParams.GL_MultiSampleBuffers);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, m_windowParams.GL_MultiSamplesSamples);

	SDL_SetWindowFullscreen(m_window, m_windowParams.Fullscreen);
	SDL_SetWindowResizable(m_window, SDL_TRUE);

	SDL_GL_SetSwapInterval(m_windowParams.VSync);
}

const SDL_DisplayMode* SDLHandler::GetCurrentDisplayMode()
{
	SDL_DisplayMode* displayMode = nullptr;
	const int displayModesSize = static_cast<int>(m_displayModes.size());
	if (m_windowParams.ResolutionIndex >= 0 && m_windowParams.ResolutionIndex < displayModesSize)
	{
		displayMode = &m_displayModes[m_windowParams.ResolutionIndex];
	}
	return displayMode;
}

const void SDLHandler::OnExitWindow(std::function<void(void)> callback)
{
	m_onExitWindowCallback = callback;
}

void SDLHandler::RenderUI()
{
	auto displayModes = GetDisplayModes();
	const unsigned int size = static_cast<unsigned int>(displayModes.size());
	std::vector<std::string> displayNamesStr;

	for (unsigned int i = 0; i < size; ++i)
	{
		const std::string name = GetDisplayModeName(i);
		displayNamesStr.push_back(name);
	}

	ImGui::Begin("Window Parameters");

	ImGui::Combo("Resolutions", &m_tempWindowParams.ResolutionIndex, displayNamesStr);
	ImGui::Checkbox("Fullscreen", &m_tempWindowParams.Fullscreen);
	ImGui::Checkbox("VSync", &m_tempWindowParams.VSync);
	ImGui::DragInt("FPS Limit", &m_tempWindowParams.FPSLimit, 1.0f, 30, 200);

	ImGui::SliderInt("Multi sample Buffers", &m_tempWindowParams.GL_MultiSampleBuffers, 1, 4);
	ImGui::SliderInt("Multi sample Samples", &m_tempWindowParams.GL_MultiSamplesSamples, 1, 32);

	if (ImGui::Button("Apply Changes", ImVec2(140, 30)))
	{
		const bool resolutionChanged = m_tempWindowParams.ResolutionIndex != m_windowParams.ResolutionIndex
			|| m_tempWindowParams.VSync != m_windowParams.VSync
			|| m_tempWindowParams.Fullscreen != m_windowParams.Fullscreen;

		const bool globalSettingsChanged = m_tempWindowParams.FPSLimit != m_windowParams.FPSLimit
			|| m_tempWindowParams.GL_MultiSampleBuffers != m_windowParams.GL_MultiSampleBuffers
			|| m_tempWindowParams.GL_MultiSamplesSamples != m_windowParams.GL_MultiSamplesSamples;

		if (resolutionChanged)
		{
			m_tempWindowParams.Width = displayModes[m_tempWindowParams.ResolutionIndex].w;
			m_tempWindowParams.Height = displayModes[m_tempWindowParams.ResolutionIndex].h;
		}

		if (resolutionChanged || globalSettingsChanged)
		{
			m_windowParams = m_tempWindowParams;
			SetWindowParameters(m_windowParams);
		}
	}

	ImGui::End();
}

void SDLHandler::LoadWindowParams()
{
	std::string config = FileIO::ReadTextFile(s_configFileName);
	if (!config.empty())
	{
		json jsonParams = json::parse(config);
		if (!jsonParams.is_null()) {
			m_windowParams = jsonParams;
		}
	}
}

void SDLHandler::SaveWindowParams()
{
	std::string config = static_cast<json>(m_windowParams).dump(4);
	FileIO::SaveTextFile(s_configFileName, config);
}

void SDLHandler::FindDisplayModes()
{
	SDL_DisplayMode currentDisplayMode;
	SDL_GetCurrentDisplayMode(m_windowParams.DisplayIndex, &currentDisplayMode);

	const int nDisplayModes = SDL_GetNumDisplayModes(m_windowParams.DisplayIndex);
	if (nDisplayModes < 1)
	{
		LOG_ERROR("SDL_GetNumDisplayModes failed: %s\n", SDL_GetError());
		return;
	}

	for (int i = 0; i < nDisplayModes; ++i)
	{
		SDL_DisplayMode mode;
		SDL_GetDisplayMode(m_windowParams.DisplayIndex, i, &mode);

		if (mode.w == currentDisplayMode.w &&
			mode.h == currentDisplayMode.h &&
			mode.refresh_rate == currentDisplayMode.refresh_rate)
		{
			m_windowParams.ResolutionIndex = i;
		}

		std::string st = std::to_string(mode.w)
			+ std::string(" x ")
			+ std::to_string(mode.h)
			+ std::string(" @ ")
			+ std::to_string(mode.refresh_rate)
			+ std::string(" hz ")
			+ std::to_string(mode.format);

		m_displayModeIndexToName.emplace(i, st);
		m_displayModes.push_back(mode);
	}
}

