#include "FluxEngine.h"
#include "PhysicsScene.h"
#include "PhysicsSystem.h"
#include "Scenegraph\Scene.h"
#include "Rendering\Camera\Camera.h"
#include "Scenegraph\Transform.h"

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
	m_pPhysicsScene = m_pSystem->GetPhysics()->createScene(desc);
}

void PhysicsScene::OnSceneRemoved()
{
	Component::OnSceneRemoved();

	if (m_pPhysicsScene)
	{
		m_pPhysicsScene->release();
		m_pPhysicsScene = nullptr;
	}
}

void PhysicsScene::Update()
{
	m_pPhysicsScene->simulate(GameTimer::DeltaTime());
	m_pPhysicsScene->fetchResults(true);

	//Send camera location to Pvd
	/*Transform* pTransform = m_pScene->GetCamera()->GetTransform();
	m_pPhysicsScene->getScenePvdClient()->updateCamera(
		"Main Camera",
		*reinterpret_cast<const PxVec3*>(&pTransform->GetWorldPosition()),
		*reinterpret_cast<const PxVec3*>(&pTransform->GetUp()),
		*reinterpret_cast<const PxVec3*>(&pTransform->GetForward()));*/
}