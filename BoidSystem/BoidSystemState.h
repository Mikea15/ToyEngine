#pragma once

#include "Engine/State.h"

#include "Boids/Boid.h"

class Game;

#define ENTITY_COUNT 10000

class BoidSystemState
	: public State
{
public:
	BoidSystemState()
		: State()
	{}

	~BoidSystemState() override {};

	void Init(Game* game) override
	{
		gamePtr = game;
		m_renderer = gamePtr->GetRenderer();

		// configure camera
		m_camera = FlyCamera(glm::vec3(0.0f, 2.0f, 10.0f));
		m_camera.SetPerspective(glm::radians(90.0f),
			static_cast<float>(m_renderer->GetRenderWidth() / m_renderer->GetRenderHeight()),
			0.01f, 200.0f);

		m_renderer->SetCamera(&m_camera);

		m_sphere = new Sphere(32, 32);
		Material* defaultForwardMat = m_renderer->CreateMaterial("default-fwd");

		m_viewGrid = ViewportGrid(100, 100, 100, 100);

		DebugDraw::Init();
	};

	void HandleInput(SDL_Event* event) override
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
	};

	void Update(float deltaTime) override
	{
		glm::vec3 mouseWorldPos = {};
		if (m_inputGrabMouse) {
			int x, y;
			SDL_GetRelativeMouseState(&x, &y);
			m_camera.HandleMouse(static_cast<float>(x), static_cast<float>(-y));

			mouseWorldPos = m_camera.ScreenSpaceToWorldSpace(
				glm::vec3(static_cast<float>(x), static_cast<float>(-y), 0.f)
			);
		}

		// get camera movement input
		glm::vec3 inputDir(m_inputMoveRight, m_inputMoveUp, m_inputMoveForward);
		m_camera.HandleMove(deltaTime, inputDir, m_inputEnableMovementBoost);

		m_camera.Update(deltaTime);

		// Debug
		DebugDraw::Clear();

		glm::mat4 viewProj = m_camera.GetViewProjection();
		DebugDraw::Update(viewProj);

		time += deltaTime;
		stopTimer -= deltaTime;
		
		if (stopTimer < 0.0f)
		{
			stopTimer = stopInterval;
			m_movingTarget = glm::normalize(glm::vec3(
				1.0f - sin(time) * 2.0f, 
				1.0f - cos(time) * 2.0f, 
				0.0f
			)) * 4.0f;
		}

		DebugDraw::AddPosition(m_movingTarget, 0.5f, { 0.6f, 0.2f, 0.4f, 1.0f });

		m_viewGrid.Draw();

		// m_simpleBoid.Seek(m_movingTarget);
		m_simpleBoid.Arrive(m_movingTarget);
		m_simpleBoid.Update(deltaTime);

		DebugDraw::AddPosition(m_simpleBoid.m_position, 0.5f, { 0.2f, 0.5f, 0.5f, 1.0f });
		DebugDraw::AddLine(m_simpleBoid.m_position, m_simpleBoid.m_position + m_simpleBoid.m_velocity, { 0.0f, 0.0f, 1.0f, 1.0f });
		DebugDraw::AddAABB(m_simpleBoid.m_position - glm::vec3(m_simpleBoid.m_radius), m_simpleBoid.m_position + glm::vec3(m_simpleBoid.m_radius), { 0.2f, 0.5f, 0.5f, 1.0f });
	};

	void Render(float alpha = 1.0f) override
	{

#if 0
		for (SceneNode* node : m_sceneNodes)
		{
			m_renderer->PushRender(node);
		}
#endif
		m_renderer->RenderPushedCommands();

		DebugDraw::AddPosition(glm::vec3(0.0f), 0.5f);

		bool x_ray = false;
		DebugDraw::Draw(x_ray);
	};

	void RenderUI() {};
	void Cleanup() override {};

private:
	Game* gamePtr;
	SimpleRenderer* m_renderer;
	FlyCamera m_camera;
	ViewportGrid m_viewGrid;
	// render representation
	Sphere* m_sphere;

	bool m_inputGrabMouse = false;
	float m_inputMoveUp = 0.0f;
	float m_inputMoveRight = 0.0f;
	float m_inputMoveForward = 0.0f;
	bool m_inputEnableMovementBoost = false;

	Boid m_simpleBoid;
	glm::vec3 m_movingTarget = { 1.0f, 1.0f, 1.0f };

	float time = {};
	float stopTimer = {};
	float stopInterval = 5.0f;
};
