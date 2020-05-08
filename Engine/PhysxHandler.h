
#pragma once

#include "Engine/Vector.h"

#include <physx/PxPhysicsAPI.h>
#include <physx/pvd/PxPvd.h>

using namespace physx;

// PhysX Docs
// https://gameworksdocs.nvidia.com/PhysX/4.1/documentation/physxguide/Index.html
// https://gameworksdocs.nvidia.com/PhysX/4.1/documentation/physxapi/

#define PVD_HOST "127.0.0.1"
#define PVD_PORT 5425
#define PVD_TIMEOUT_MS 10


#define Vec3ToPx(v) PxVec3(v.x, v.y, v.z)
#define PxToVec3(v) glm::vec3(v.x, v.y, v.z)

class PhysXHandler
{
public:
	PhysXHandler() = default;

	struct Params
	{
		unsigned int m_threads = 2;
		glm::vec3 m_gravity = { 0.0f, -9.81f, 0.0f };

	};

	void Init();
	void Cleanup();

	void Tick(float deltaTime);


	void CreateDemoScene();

	PxRigidDynamic* CreateDynamic(const PxTransform& t, const PxGeometry& geometry, const PxVec3& velocity = PxVec3(0));
	void CreateStack(const PxTransform& t, PxU32 size, PxReal halfExtent);
	PxPhysics* CustomCreatePhysics(PxU32 version, PxFoundation& foundation, const PxTolerancesScale& scale,
		bool trackOutstandingAllocations, PxPvd* pvd);

private:
	PxDefaultAllocator		gAllocator;
	PxDefaultErrorCallback	gErrorCallback;

	PxFoundation* m_foundation = nullptr;
	PxPhysics* m_physics = nullptr;

	PxDefaultCpuDispatcher* gDispatcher = nullptr;
	PxScene* m_mainScene = nullptr;

	PxMaterial* m_defaultMaterial = nullptr;

	PxPvd* m_visualDebugger = nullptr;

	Params m_defaults;
};