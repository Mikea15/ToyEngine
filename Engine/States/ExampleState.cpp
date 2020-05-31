#include "ExampleState.h"

#include "Game.h"
#include "Mesh/Cube.h"
#include "Mesh/PlaneMesh.h"
#include "Mesh/Sphere.h"
#include "Mesh/Torus.h"
#include "Renderer/SimpleRenderer.h"
#include "Resources/Resources.h"
#include "Scene/SceneNode.h"
#include "Scene/Scene.h"
#include "Shading/Material.h"

#include <physx/PxRigidDynamic.h>

void ExampleState::Init(Game* game)
{
    BaseState::Init(game);

    // debug draw init
    DebugDraw::Init();

    // basic shapes
    plane = new PlaneMesh(50, 50);
    sphere = new Sphere(32, 32);
    tSphere = new Sphere(256, 256);
    torus = new Torus(2.0f, 0.4f, 32, 32);
    cube = new Cube();

    // material setup
    Material* matPbr = m_renderer->CreateMaterial("default-fwd");
    Shader* plasmaOrbShader = Resources::LoadShader("plasma orb", "shaders/custom/plasma_orb.vs", "shaders/custom/plasma_orb.fs");
    Material* matPlasmaOrb = m_renderer->CreateCustomMaterial(plasmaOrbShader);
    matPlasmaOrb->Cull = false;
    matPlasmaOrb->Blend = true;
    matPlasmaOrb->BlendSrc = GL_ONE;
    matPlasmaOrb->BlendDst = GL_ONE;
    matPlasmaOrb->SetTexture("TexPerllin", Resources::LoadTexture("perlin noise", "textures/perlin.png"), 0);
    matPlasmaOrb->SetFloat("Strength", 1.5f);
    matPlasmaOrb->SetFloat("Speed", 0.083f);

    // configure camera
    m_camera.SetPerspective(glm::radians(90.0f),
        static_cast<float>(m_renderer->GetRenderWidth() / m_renderer->GetRenderHeight()),
        0.01f, 200.0f);
    m_camera.SetPosition(glm::vec3(0.0f, 0.0f, 5.0f));

    // scene setup
    debugShader = Resources::LoadShader("debugShader", "shaders/error.vs", "shaders/error.fs");
    Material* defaultForwardMat = m_renderer->CreateMaterial("default-fwd");
    Material* defaultForwardMatAlpha = m_renderer->CreateMaterial("default-fwd-alpha");
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
    m_oTree = AABBOctree(glm::vec3(0.0f), 50.0f);

    const float spacing = 7.2f;
    for (int x = 0; x < 5; ++x)
    {
        for (int z = 0; z < 5; ++z)
        {
            for (int y = 0; y < 5; ++y)
            {
                glm::vec3 position = glm::vec3(0.0f, 0.5f, 0.0f) + glm::vec3(x - 5, y, z - 5) * spacing;

                float rand = MathUtils::Rand01();
                SceneNode* node = Scene::MakeSceneNode(tSphere, rand < 1.0f ? defaultForwardMat : defaultForwardMatAlpha);
                node->SetPosition(position);

                const float randomScale = MathUtils::Rand(0.5f, 2.3f);
                node->SetScale(randomScale);

                m_randomNodes.push_back(node);

                auto dynObject = m_game->GetPhysX()->CreateDynamic(
                    PxTransform(PxVec3(position.x, position.y, position.z)),
                    PxSphereGeometry(randomScale), PxVec3(0.0f, 3.0f * randomScale, 0.0f));
                m_dynamicObjects.push_back(dynObject);

                glm::vec3 min = { position.x + node->BoxMin.x * randomScale, position.y + node->BoxMin.y * randomScale, position.z + node->BoxMin.z * randomScale };
                glm::vec3 max = { position.x + node->BoxMax.x * randomScale, position.y + node->BoxMax.y * randomScale, position.z + node->BoxMax.z * randomScale };
                glm::vec4 green = { 0.0f, 1.0f, 0.0f, 1.0f };

                // DebugDraw::AddAABB(min, max, green);

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

    m_renderer->AddLight(&m_directionalLight);

    m_viewGrid = ViewportGrid(100, 100, 100, 100);

    auto camFrustum = m_camera.GetFrustum();
    DebugDraw::AddFrustrum(camFrustum.FTL, camFrustum.FTR, camFrustum.FBL, camFrustum.FBR,
        camFrustum.NTL, camFrustum.NTR, camFrustum.NBL, camFrustum.NBR, { 1.0f, 0.2f, 0.2f, 1.0f });

    //

    //
}

void ExampleState::HandleInput(SDL_Event* event)
{
    BaseState::HandleInput(event);
}

void ExampleState::Update(float deltaTime)
{
    BaseState::Update(deltaTime);

    mainTorus->SetRotation(glm::vec4(glm::vec3(1.0f, 0.0f, 0.0f), m_game->GetTotalTime() * 10.0f * 2.0f));
    secondTorus->SetRotation(glm::vec4(glm::vec3(0.0f, 1.0f, 0.0f), m_game->GetTotalTime() * 10.0f * 3.0f));
    thirdTorus->SetRotation(glm::vec4(glm::vec3(0.0f, 1.0f, 0.0f), m_game->GetTotalTime() * 10.0f * 4.0f));

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
        m_oTree.DebugDraw();
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
        m_viewGrid.PushDraw();
    }

    auto camFrustum = m_camera.GetFrustum();
    DebugDraw::AddFrustrum(camFrustum.FTL, camFrustum.FTR, camFrustum.FBL, camFrustum.FBR,
        camFrustum.NTL, camFrustum.NTR, camFrustum.NBL, camFrustum.NBR, { 1.0f, 0.2f, 0.2f, 1.0f });

    glm::mat4 viewProj = m_camera.GetViewProjection();
    DebugDraw::Update(viewProj);

    // Update PhysX Objects
    const unsigned int objectSize = m_dynamicObjects.size();
    for (unsigned int i = 0; i < objectSize; i++)
    {
        physx::PxRigidDynamic* dyn = m_dynamicObjects[i];

        glm::vec3 pos = { dyn->getGlobalPose().p.x,
            dyn->getGlobalPose().p.y,
            dyn->getGlobalPose().p.z };

        SceneNode* node = m_randomNodes[i];
        node->SetPosition(pos);
    }
}

void ExampleState::Render(float alpha /*= 1.0f*/)
{
    if (m_drawObjects)
    {
        // renderer->PushRender(planeNode);
        // renderer->PushRender(mainTorus);
        // renderer->PushRender(sponza);
        // renderer->PushRender(plasmaOrb);
        // renderer->PushRender(background);

        for (SceneNode* node : m_randomNodes)
        {
            m_renderer->PushRender(node);
        }
    }

    m_renderer->RenderPushedCommands();

    DebugDraw::AddPosition(m_camera.WorldSpaceToScreenSpace(glm::vec3(0.0f)), 0.1f);

    DebugDraw::AddPosition(glm::vec3(0.0f), 0.5f);
    DebugDraw::AddLine(glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
    DebugDraw::AddLine(glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
    DebugDraw::AddLine(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
    bool x_ray = false;
    DebugDraw::Draw(x_ray);
}

void ExampleState::RenderUI()
{
    BaseState::RenderUI();
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
            ImGui::Checkbox("Draw AABBOctree", &m_drawOctree);
            ImGui::Checkbox("Draw BVH", &m_drawBVH);
            ImGui::Checkbox("Draw Grid", &m_drawGrid);
            ImGui::EndMenu();
        }
        m_renderer->RenderUIMenu();
        ImGui::EndMainMenuBar();
    }
    Scene::DrawSceneUI();
}

void ExampleState::Cleanup()
{
    BaseState::Cleanup();
    // free db drawing memory
    DebugDraw::Clean();
}
