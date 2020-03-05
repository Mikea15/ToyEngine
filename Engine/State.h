#pragma once

union SDL_Event;

#include "Game.h"
#include "Renderer/SimpleRenderer.h"
#include "Renderer/ViewportGrid.h"
#include "Scene/Skybox.h"
#include "Shading/Material.h"
#include "Shading/Shader.h"
#include "Resources/Resources.h"
#include "Scene/Scene.h"
#include "Scene/SceneNode.h"
#include "Lighting/DirectionalLight.h"
#include "Mesh/PlaneMesh.h"
#include "Mesh/Sphere.h"
#include "Mesh/Torus.h"
#include "Mesh/Cube.h"

#include "Systems/QuadTree.h"
#include "Systems/Octree.h"

#include "Utils/MathUtils.h"
#include "Renderer/DebugDraw.h"

#include "Systems/BTree.h"
#include "Systems/BST.h"
#include "Systems/BVH.h"

#include <stack>

#include <physx/PxPhysicsAPI.h>

using namespace physx;

class State
{
public:
	virtual ~State() = default;

	virtual void Init(Game* game) = 0;
	virtual void HandleInput(SDL_Event* event) = 0;
	virtual void Update(float deltaTime) = 0;
	virtual void Render(float alpha = 1.0f) = 0;

	virtual void RenderUI() {};
	virtual void Cleanup() = 0;
};

class PxAllocatorCallback
{
public:
	virtual ~PxAllocatorCallback() {}
	virtual void* allocate(size_t size, const char* typeName, const char* filename,
		int line) = 0;
	virtual void deallocate(void* ptr) = 0;
};

class UserErrorCallback : public PxErrorCallback
{
public:
	virtual void reportError(PxErrorCode::Enum code, const char* message, const char* file,
		int line)
	{
		// error processing implementation
	}
};

class StubState
	: public State
{
public:
	StubState()
		: State()
	{}

	~StubState() override {};

	void Init(Game* game) override 
	{
		m_game = game;
		renderer = m_game->GetRenderer();
		renderer->SetCamera(&m_camera);

		// debug draw init
		DebugDraw::Init();

		static PxDefaultErrorCallback gDefaultErrorCallback;
		static PxDefaultAllocator gDefaultAllocatorCallback;

		auto mFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gDefaultAllocatorCallback,
			gDefaultErrorCallback);
		if (!mFoundation)
			printf("PxCreateFoundation failed!");

		// basic shapes
		// plane = new PlaneMesh(50, 50);
		sphere = new Sphere(32, 32);
		tSphere = new Sphere(256, 256);
		torus = new Torus(2.0f, 0.4f, 32, 32);
		cube = new Cube();

		// material setup
		Material* matPbr = renderer->CreateMaterial("default-fwd");
		Shader* plasmaOrbShader = Resources::LoadShader("plasma orb", "shaders/custom/plasma_orb.vs", "shaders/custom/plasma_orb.fs");
		Material* matPlasmaOrb = renderer->CreateCustomMaterial(plasmaOrbShader);
		matPlasmaOrb->Cull = false;
		matPlasmaOrb->Blend = true;
		matPlasmaOrb->BlendSrc = GL_ONE;
		matPlasmaOrb->BlendDst = GL_ONE;
		matPlasmaOrb->SetTexture("TexPerllin", Resources::LoadTexture("perlin noise", "textures/perlin.png"), 0);
		matPlasmaOrb->SetFloat("Strength", 1.5f);
		matPlasmaOrb->SetFloat("Speed", 0.083f);

		// configure camera
		m_camera.SetPerspective(glm::radians(90.0f),
			static_cast<float>(renderer->GetRenderWidth() / renderer->GetRenderHeight()),
			0.01f, 200.0f);
		m_camera.SetPosition(glm::vec3(0.0f, 0.0f, 5.0f));

		// scene setup
		debugShader = Resources::LoadShader("debugShader", "shaders/error.vs", "shaders/error.fs");
		Material* defaultForwardMat = renderer->CreateMaterial("default-fwd");
		Material* defaultForwardMatAlpha = renderer->CreateMaterial("default-fwd-alpha");
		Material* debugShaderMat = new Material(debugShader);
		mainTorus = Scene::MakeSceneNode(torus, defaultForwardMat);
		secondTorus = Scene::MakeSceneNode(torus, defaultForwardMat);
		thirdTorus = Scene::MakeSceneNode(torus, defaultForwardMat);
		plasmaOrb = Scene::MakeSceneNode(tSphere, defaultForwardMat);
		planeNode = Scene::MakeSceneNode(plane, defaultForwardMat);

		mainTorus->AddChild(secondTorus);
		secondTorus->AddChild(thirdTorus);

		mainTorus->SetScale(1.0f);
		mainTorus->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
		secondTorus->SetScale(0.65f);
		secondTorus->SetRotation(glm::vec4(0.0, 1.0, 0.0, 90.0f));
		thirdTorus->SetScale(0.65f);

		planeNode->SetPosition(glm::vec3(0.0f));
		planeNode->SetScale(10.0f);
		planeNode->SetRotation(glm::vec4(1.0f, 0.0f, 0.0f, 90.f));

		plasmaOrb->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
		plasmaOrb->SetScale(0.6f);

		m_qTree = QuadTree(glm::vec3(0.0f), 50.0f);
		m_oTree = Octree(glm::vec3(0.0f), 50.0f);

		const float spacing = 7.2f;
		for (int x = 0; x < 10; ++x)
		{
			for (int z = 0; z < 10; ++z)
			{
				for (int y = 0; y < 10; ++y)
				{
					glm::vec3 position = glm::vec3(0.0f, 0.5f, 0.0f) + glm::vec3(x - 5, y, z - 5) * spacing;

					float rand = MathUtils::Rand01();
					SceneNode* node = Scene::MakeSceneNode(tSphere, rand < 1.0f ? defaultForwardMat : defaultForwardMatAlpha);
					node->SetPosition(position);

					const float randomScale = MathUtils::Rand(0.5f, 2.3f);
					node->SetScale(randomScale);

					m_randomNodes.push_back(node);

					glm::vec3 min = { position.x + node->BoxMin.x * randomScale, position.y + node->BoxMin.y * randomScale, position.z + node->BoxMin.z * randomScale};
					glm::vec3 max = { position.x + node->BoxMax.x * randomScale, position.y + node->BoxMax.y * randomScale, position.z + node->BoxMax.z * randomScale};
					glm::vec4 green = { 0.0f, 1.0f, 0.0f, 1.0f };

					DebugDraw::AddAABB(min, max, green);

					m_qTree.Insert(position);
					m_oTree.Insert(position);

					m_bvhTree.InsertNode(x * z * y, AABB(min, max));
				}
			}
		}

		//// - background
		// Skybox* background = new Skybox();
		//PBRCapture* pbrEnv = rendererPtr->GetSkypCature();
		//background->SetCubemap(pbrEnv->Prefiltered);
		//float lodLevel = 1.5f;
		//background->Material->SetFloat("lodLevel", lodLevel);

		// post processing
		// Shader* postProcessing1 = Resources::LoadShader("postprocessing1", "shaders/screen_quad.vs", "shaders/custom_post_1.fs");
		// Shader* postProcessing2 = Resources::LoadShader("postprocessing2", "shaders/screen_quad.vs", "shaders/custom_post_2.fs");
		// Material* customPostProcessing1 = renderer->CreatePostProcessingMaterial(postProcessing1);
		// Material* customPostProcessing2 = renderer->CreatePostProcessingMaterial(postProcessing2);

		// mesh 
		//sponza = Resources::LoadMesh(renderer, "sponza", "meshes/sponza/sponza.obj");
		//sponza->SetPosition(glm::vec3(0.0, -1.0, 0.0));
		//sponza->SetScale(0.01f);

		m_directionalLight.m_direction = -glm::normalize(glm::vec3(0.0f, 10.0f, 0.0f));
		m_directionalLight.m_intensity = 10.0f;
		m_directionalLight.m_color = glm::vec3(0.9f, 0.8f, 0.8f);

		renderer->AddLight(&m_directionalLight);

		m_viewGrid = ViewportGrid(100, 100, 100, 100);

		auto camFrustum = m_camera.GetFrustum();
		DebugDraw::AddFrustrum(camFrustum.FTL, camFrustum.FTR, camFrustum.FBL, camFrustum.FBR,
			camFrustum.NTL, camFrustum.NTR, camFrustum.NBL, camFrustum.NBR, {1.0f, 0.2f, 0.2f, 1.0f});

		//
		
		//
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
		if (m_inputGrabMouse) {
			int x, y;
			SDL_GetRelativeMouseState(&x, &y);
			m_camera.HandleMouse(static_cast<float>(x), static_cast<float>(-y));
		}

		// get camera movement input
		glm::vec3 inputDir(m_inputMoveRight, m_inputMoveUp, m_inputMoveForward);
		m_camera.HandleMove(deltaTime, inputDir, m_inputEnableMovementBoost);

		m_camera.Update(deltaTime);

		mainTorus->SetRotation(glm::vec4(glm::vec3(1.0f, 0.0f, 0.0f), m_game->GetTimeMS() * 10.0f * 2.0f));
		secondTorus->SetRotation(glm::vec4(glm::vec3(0.0f, 1.0f, 0.0f), m_game->GetTimeMS() * 10.0f * 3.0f));
		thirdTorus->SetRotation(glm::vec4(glm::vec3(0.0f, 1.0f, 0.0f), m_game->GetTimeMS() * 10.0f * 4.0f));

		// Debug
		DebugDraw::Clear();

		if (m_drawQuadtree) 
		{
			// draw quadtree
			std::vector<Rect> quadTreeVis;
			m_qTree.GetAllBoundingBoxes(quadTreeVis);
			const unsigned int qSize = static_cast<unsigned int>(quadTreeVis.size());
			for (unsigned int i = 0; i < qSize; ++i)
			{
				auto pos2D = quadTreeVis[i].GetPosition();
				auto min = quadTreeVis[i].GetMin();
				auto max = quadTreeVis[i].GetMax();

				DebugDraw::AddRect(min, max, { 0.2f, 0.8f, 0.4f, 1.0f });
			}
		}

		if (m_drawOctree) 
		{
			// draw octree
			std::vector<AABB> octreeVis;
			m_oTree.GetAllBoundingBoxes(octreeVis);
			const unsigned int oSize = static_cast<unsigned int>(octreeVis.size());
			for (unsigned int i = 0; i < oSize; ++i)
			{
				auto pos = octreeVis[i].GetPosition();
				auto min = octreeVis[i].GetMin();
				auto max = octreeVis[i].GetMax();
				DebugDraw::AddAABB(min, max);
			}
		}

		if (m_drawBVH)
		{
			// draw octree
			auto nodes = m_bvhTree.GetNodes();
			const unsigned int oSize = static_cast<unsigned int>(nodes.size());
			for (unsigned int i = 0; i < oSize; ++i)
			{
				auto box = nodes[i]->box;
				DebugDraw::AddAABB(box.GetMin(), box.GetMax());
			}
		}

		// draw grid
		if (m_drawGrid) 
		{
			m_viewGrid.Draw();
		}

		auto camFrustum = m_camera.GetFrustum();
		DebugDraw::AddFrustrum(camFrustum.FTL, camFrustum.FTR, camFrustum.FBL, camFrustum.FBR,
			camFrustum.NTL, camFrustum.NTR, camFrustum.NBL, camFrustum.NBR, { 1.0f, 0.2f, 0.2f, 1.0f });

		glm::mat4 viewProj = m_camera.GetViewProjection();
		DebugDraw::Update(viewProj);
	};

	void Render(float alpha = 1.0f) override 
	{
		if (m_drawObjects) 
		{
			// renderer->PushRender(planeNode);
			// renderer->PushRender(mainTorus);
			//renderer->PushRender(sponza);
			//renderer->PushRender(plasmaOrb);
			//renderer->PushRender(background);

			for (SceneNode* node : m_randomNodes)
			{
				renderer->PushRender(node);
			}
		}

		renderer->RenderPushedCommands();

		DebugDraw::AddPosition(m_camera.WorldSpaceToScreenSpace(glm::vec3(0.0f)), 0.1f);

		DebugDraw::AddPosition(glm::vec3(0.0f), 0.5f);
		DebugDraw::AddLine(glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
		DebugDraw::AddLine(glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
		DebugDraw::AddLine(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
		bool x_ray = false;
		DebugDraw::Draw(x_ray);
	};

	void RenderUI() 
	{
		// Menu Bar
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				// ShowExampleMenuFile();
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Edit"))
			{
				if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
				if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
				ImGui::Separator();
				if (ImGui::MenuItem("Cut", "CTRL+X")) {}
				if (ImGui::MenuItem("Copy", "CTRL+C")) {}
				if (ImGui::MenuItem("Paste", "CTRL+V")) {}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Viewport")) {
				ImGui::Checkbox("Draw Objects", &m_drawObjects);
				ImGui::Checkbox("Draw Quadtree", &m_drawQuadtree);
				ImGui::Checkbox("Draw Octree", &m_drawOctree);
				ImGui::Checkbox("Draw BVH", &m_drawBVH);
				ImGui::Checkbox("Draw Grid", &m_drawGrid);
				ImGui::EndMenu();
			}
			renderer->RenderUIMenu();
			ImGui::EndMainMenuBar();
		}

		Scene::DrawSceneUI();
	};
	void Cleanup() override {
		// free db drawing memory
		DebugDraw::Clean();
	};

private:
	Game* m_game;
	SimpleRenderer* renderer;
	FlyCamera m_camera = FlyCamera(glm::vec3(0.0f, 5.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
	
	PlaneMesh* plane;
	Sphere*	sphere;
	Sphere*	tSphere;
	Torus* torus;
	Cube* cube;

	Shader* debugShader;

	SceneNode* sponza;
	SceneNode* mainTorus;
	SceneNode* secondTorus;
	SceneNode* thirdTorus;
	SceneNode* plasmaOrb;
	SceneNode* planeNode;

	DirectionalLight m_directionalLight;

	std::vector<SceneNode*> m_randomNodes;

	bool m_inputGrabMouse = false;
	float m_inputMoveUp = 0.0f;
	float m_inputMoveRight = 0.0f;
	float m_inputMoveForward = 0.0f;
	bool m_inputEnableMovementBoost = false;

	bool m_drawObjects = false;
	bool m_drawQuadtree = false;
	bool m_drawOctree = false;
	bool m_drawBVH = false;
	bool m_drawGrid = true;

	QuadTree m_qTree;
	Octree m_oTree;
	ViewportGrid m_viewGrid;

	bvh::Tree m_bvhTree;
	// BTree<int> btree;
	
};
