#include "FluxEngine.h"
#include "PhysicsScene.h"
#include "PhysicsSystem.h"

using namespace physx;

PhysicsScene::PhysicsScene(PhysicsSystem* pSystem) :
	m_pSystem(pSystem)
{

}

PhysicsScene::~PhysicsScene()
{
}

void PhysicsScene::OnSceneSet(Scene* pScene)
{
	Component::OnSceneSet(pScene);

	PxSceneDesc desc(m_pSystem->GetPhysics()->getTolerancesScale());
	desc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
	desc.cpuDispatcher = m_pSystem->GetCpuDispatcher();
	desc.gpuDispatcher = nullptr;//m_pCudaContextManager ? m_pCudaContextManager->getGpuDispatcher() : nullptr;
	desc.filterShader = PhysicsSystem::SimulationFilterShader;
	desc.userData = this;
	m_pScene = m_pSystem->GetPhysics()->createScene(desc);
}

void PhysicsScene::OnSceneRemoved()
{
	Component::OnSceneRemoved();

	if (m_pScene)
	{
		m_pScene->release();
		m_pScene = nullptr;
	}
}

void PhysicsScene::Update()
{
	m_pScene->simulate(GameTimer::DeltaTime());
	if (!m_pScene->fetchResults(true))
	{
	}
}