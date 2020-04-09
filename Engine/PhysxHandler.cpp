#include "PhysxHandler.h"

void PhysXHandler::Init()
{
	m_foundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);

	m_visualDebugger = PxCreatePvd(*m_foundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, PVD_PORT, PVD_TIMEOUT_MS);
	m_visualDebugger->connect(*transport, PxPvdInstrumentationFlag::eALL);

	m_physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_foundation, PxTolerancesScale(), true, m_visualDebugger);

	// PxRegisterArticulations(*m_physics);
	// PxRegisterHeightFields(*m_physics);

	gDispatcher = PxDefaultCpuDispatcherCreate(m_defaults.m_threads);

	PxSceneDesc sceneDesc(m_physics->getTolerancesScale());
	sceneDesc.gravity = Vec3ToPx(m_defaults.m_gravity);
	sceneDesc.cpuDispatcher = gDispatcher;
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;
	m_mainScene = m_physics->createScene(sceneDesc);

	PxPvdSceneClient* pvdClient = m_mainScene->getScenePvdClient();
	if (pvdClient)
	{
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}

	m_defaultMaterial = m_physics->createMaterial(0.5f, 0.5f, 0.6f);
}

void PhysXHandler::Cleanup()
{
}

void PhysXHandler::Tick(float deltaTime)
{
	m_mainScene->simulate(deltaTime);
	m_mainScene->fetchResults(true);
}

void PhysXHandler::CreateDemoScene()
{
	PxRigidStatic* groundPlane = PxCreatePlane(*m_physics, PxPlane(0, 1, 0, 0), *m_defaultMaterial);
	m_mainScene->addActor(*groundPlane);
}

PxRigidDynamic* PhysXHandler::CreateDynamic(const PxTransform& t, const PxGeometry& geometry, const PxVec3& velocity)
{
	PxRigidDynamic* dynamic = PxCreateDynamic(*m_physics, t, geometry, *m_defaultMaterial, 10.0f);
	dynamic->setAngularDamping(0.5f);
	dynamic->setLinearVelocity(velocity);
	m_mainScene->addActor(*dynamic);
	return dynamic;
}

void PhysXHandler::CreateStack(const PxTransform& t, PxU32 size, PxReal halfExtent)
{
	PxShape* shape = m_physics->createShape(PxBoxGeometry(halfExtent, halfExtent, halfExtent), *m_defaultMaterial);
	for (PxU32 i = 0; i < size; i++)
	{
		for (PxU32 j = 0; j < size - i; j++)
		{
			PxTransform localTm(PxVec3(PxReal(j * 2) - PxReal(size - i), PxReal(i * 2 + 1), 0) * halfExtent);
			PxRigidDynamic* body = m_physics->createRigidDynamic(t.transform(localTm));
			body->attachShape(*shape);
			PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
			m_mainScene->addActor(*body);
		}
	}
	shape->release();
}

PxPhysics* PhysXHandler::CustomCreatePhysics(PxU32 version, PxFoundation& foundation, const PxTolerancesScale& scale, bool trackOutstandingAllocations, PxPvd* pvd)
{
	return nullptr;
}

