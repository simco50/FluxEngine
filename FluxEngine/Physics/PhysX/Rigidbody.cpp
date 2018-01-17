#include "FluxEngine.h"
#include "Rigidbody.h"
#include "Scenegraph\Transform.h"
#include "PhysicsSystem.h"
#include "PhysicsScene.h"
#include "Scenegraph\SceneNode.h"
#include "Scenegraph\Scene.h"

Rigidbody::Rigidbody(PhysicsSystem* pSystem) : 
	m_pSystem(pSystem)
{

}

Rigidbody::~Rigidbody()
{
}

void Rigidbody::OnSceneSet(Scene* pScene)
{
	Component::OnSceneSet(pScene);

	m_pPhysicsScene = pScene->GetOrCreateComponent<PhysicsScene>(m_pSystem);
	m_pPhysicsScene->GetScene()->addActor(*m_pBody);
}

void Rigidbody::OnSceneRemoved()
{
	Component::OnSceneRemoved();

	if (m_pPhysicsScene && m_pBody)
	{
		m_pPhysicsScene->GetScene()->removeActor(*m_pBody);
		m_pBody = nullptr;
	}
}

void Rigidbody::OnNodeSet(SceneNode* pNode)
{
	Component::OnNodeSet(pNode);

	Transform* pTransform = GetTransform();
	PxTransform transform = *reinterpret_cast<const PxTransform*>(&pTransform->GetWorldMatrix());
	if (m_Dynamic)
		m_pBody = m_pSystem->GetPhysics()->createRigidDynamic(transform);
	else
		m_pBody = m_pSystem->GetPhysics()->createRigidStatic(transform);
	m_pBody->userData = this;
}

void Rigidbody::OnNodeRemoved()
{
	Component::OnNodeRemoved();

	if (m_pPhysicsScene && m_pBody)
	{
		m_pPhysicsScene->GetScene()->removeActor(*m_pBody);
		m_pBody = nullptr;
	}
}
