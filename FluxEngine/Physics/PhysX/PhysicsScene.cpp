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

	PxTolerancesScale scale = PxTolerancesScale();
	PxSceneDesc desc(scale);
	desc.gravity.y = 9.81f;
	//m_pScene = m_pSystem->GetPhysics()->createScene(desc);
}

void PhysicsScene::OnSceneRemoved()
{
	Component::OnSceneRemoved();

	if (m_pScene != nullptr)
		m_pScene->release();
}

void PhysicsScene::Update()
{
	m_pScene->simulate(GameTimer::DeltaTime());
	if (!m_pScene->fetchResults())
	{
	}
}