#pragma once

#include "BaseState.h"

#include "Core/AABBOctree.h"
#include "Lighting/DirectionalLight.h"
#include "Renderer/ViewportGrid.h"
#include "Systems/BVH.h"
#include "Systems/QuadTree.h"

#include <vector>

class PlaneMesh;
class Sphere;
class Torus;
class Cube;
class Shader;
class SceneNode;

namespace physx 
{
    class PxRigidDynamic;
}

class ExampleState
    : public BaseState
{
public:
    ExampleState()
        : BaseState()
    {}

    ~ExampleState() override {};

    void Init(Game* game) override;
    void HandleInput(SDL_Event* event) override;
    void Update(float deltaTime) override;
    void UpdatePaused(float deltaTime) override {}
    void Render(float alpha = 1.0f) override;
    void RenderUI() override;
    void Cleanup() override;

private:
    PlaneMesh* plane = nullptr;
    Sphere* sphere = nullptr;
    Sphere* tSphere = nullptr;
    Torus* torus = nullptr;
    Cube* cube = nullptr;

    Shader* debugShader = nullptr;

    SceneNode* sponza = nullptr;
    SceneNode* mainTorus = nullptr;
    SceneNode* secondTorus = nullptr;
    SceneNode* thirdTorus = nullptr;
    SceneNode* plasmaOrb = nullptr;
    SceneNode* planeNode = nullptr;

    DirectionalLight m_directionalLight;

    std::vector<SceneNode*> m_randomNodes;
    std::vector<physx::PxRigidDynamic*> m_dynamicObjects;

    bool m_drawObjects = true;
    bool m_drawQuadtree = false;
    bool m_drawOctree = false;
    bool m_drawBVH = false;
    bool m_drawGrid = true;

    QuadTree m_qTree;
    AABBOctree m_oTree;
    ViewportGrid m_viewGrid;

    bvh::Tree m_bvhTree;
    // BTree<int> btree;
};