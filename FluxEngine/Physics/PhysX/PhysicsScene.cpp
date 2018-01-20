#include "FluxEngine.h"
#include "PhysicsScene.h"
#include "PhysicsSystem.h"
#include "Scenegraph\Scene.h"
#include "Rendering\Camera\Camera.h"
#include "Scenegraph\Transform.h"
#include "Rigidbody.h"

using namespace physx;

const float PhysicsScene::PHYSICS_TIME_STEP = 1.0f / 60.0f;

PhysicsScene::PhysicsScene(PhysicsSystem* pPhysicsSystem) :
	m_pPhysicsSystem(pPhysicsSystem)
{
}

PhysicsScene::~PhysicsScene()
{
}

void PhysicsScene::OnSceneSet(Scene* pScene)
{
	Component::OnSceneSet(pScene);

	PxCudaContextManager* pCudaContextManager = m_pPhysicsSystem->GetCudaContextManager();

	AUTOPROFILE(PhysicsScene_CreateScene);
	PxSceneDesc desc(m_pPhysicsSystem->GetPhysics()->getTolerancesScale());
	desc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
	desc.cpuDispatcher = m_pPhysicsSystem->GetCpuDispatcher();
	desc.filterShader = PhysicsSystem::SimulationFilterShader;
	desc.simulationEventCallback = this;
	desc.userData = this;
	if (pCudaContextManager)
	{
		desc.flags |= PxSceneFlag::eENABLE_GPU_DYNAMICS;
		desc.broadPhaseType = PxBroadPhaseType::eGPU;
		desc.gpuDispatcher = pCudaContextManager->getGpuDispatcher();
	}
	m_pPhysicsScene = m_pPhysicsSystem->GetPhysics()->createScene(desc);
}

void PhysicsScene::OnSceneRemoved()
{
	Component::OnSceneRemoved();

	PhysXSafeRelease(m_pPhysicsScene);
}

void PhysicsScene::Update()
{
	m_pPhysicsScene->simulate(PHYSICS_TIME_STEP);
	m_pPhysicsScene->fetchResults(true);

	//Send camera location to Pvd
	/*Transform* pTransform = m_pScene->GetCamera()->GetTransform();
	m_pPhysicsScene->getScenePvdClient()->updateCamera(
		"Main Camera",
		*reinterpret_cast<const PxVec3*>(&pTransform->GetWorldPosition()),
		*reinterpret_cast<const PxVec3*>(&pTransform->GetUp()),
		*reinterpret_cast<const PxVec3*>(&pTransform->GetForward()));*/
}

void PhysicsScene::onTrigger(PxTriggerPair* pairs, PxU32 count)
{
	for (PxU32 i = 0; i < count; i++)
	{
		// ignore pairs when shapes have been deleted
		if (pairs[i].flags & (PxTriggerPairFlag::eREMOVED_SHAPE_TRIGGER | PxTriggerPairFlag::eREMOVED_SHAPE_TRIGGER))
			continue;

		Rigidbody* pBodyA = reinterpret_cast<Rigidbody*>(pairs[i].triggerActor->userData);
		Rigidbody* pBodyB = reinterpret_cast<Rigidbody*>(pairs[i].otherActor->userData);
		if (pBodyA)
		{
			if (pairs[i].status & PxPairFlag::eNOTIFY_TOUCH_FOUND)
				pBodyA->OnTrigger().Broadcast(pBodyB, true);
			else if (pairs[i].status & PxPairFlag::eNOTIFY_TOUCH_LOST)
				pBodyA->OnTrigger().Broadcast(pBodyB, false);
		}
	}
}