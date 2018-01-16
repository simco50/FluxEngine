#include "FluxEngine.h"
#include "Rigidbody.h"
#include "Scenegraph\Transform.h"
#include "PhysicsSystem.h"

Rigidbody::Rigidbody(PhysicsSystem* pSystem) : 
	m_pSystem(pSystem)
{

}

Rigidbody::~Rigidbody()
{
	/*if (m_pBody)
	{
		m_pBody->release();
		m_pBody = nullptr;
	}*/
}

void Rigidbody::OnSceneSet(Scene* pScene)
{
	Component::OnSceneSet(pScene);

	Transform* pTransform = GetTransform();
	PxTransform transform = *reinterpret_cast<const PxTransform*>(&pTransform->GetWorldMatrix());
	if (m_Dynamic)
		m_pBody = m_pSystem->GetPhysics()->createRigidDynamic(transform);
	else
		m_pBody = m_pSystem->GetPhysics()->createRigidStatic(transform);
	m_pBody->userData = this;
}

void Rigidbody::OnSceneRemoved()
{
	Component::OnSceneRemoved();

	if (m_pBody)
	{
		m_pBody->release();
		m_pBody = nullptr;
	}
}