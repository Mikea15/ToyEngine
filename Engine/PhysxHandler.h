
#pragma once

#include <physx/PxPhysicsAPI.h>
#include <physx/pvd/PxPvd.h>

using namespace physx;

class PhysXHandler
{
public:
	PhysXHandler() = default;

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

	PxFoundation* gFoundation = nullptr;
	PxPhysics* gPhysics = nullptr;

	PxDefaultCpuDispatcher* gDispatcher = nullptr;
	PxScene* gScene = nullptr;

	PxMaterial* gMaterial = nullptr;

	PxPvd* gPvd = nullptr;

	PxReal stackZ = 10.0f;
};