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
    void Render(float alpha = 1.0f) override;
    void RenderUI() override;
    void Cleanup() override;

private:
    PlaneMesh* plane;
    Sphere* sphere;
    Sphere* tSphere;
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