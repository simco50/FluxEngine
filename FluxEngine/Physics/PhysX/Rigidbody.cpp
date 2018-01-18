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
	PxTransform transform(*reinterpret_cast<const PxVec3*>(&pTransform->GetWorldPosition()), *reinterpret_cast<const PxQuat*>(&pTransform->GetWorldRotation()));
	switch (m_Type)
	{
	case Rigidbody::Type::Static:
		m_pBody = m_pSystem->GetPhysics()->createRigidStatic(transform);
		break;
	case Rigidbody::Type::Dynamic:
		m_pBody = m_pSystem->GetPhysics()->createRigidDynamic(transform);
		break;
	}
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

void Rigidbody::OnMarkedDirty(const Matrix& worldMatrix)
{
	if (m_pBody)
	{
		m_pBody->setGlobalPose(*reinterpret_cast<const PxTransform*>(&worldMatrix), true);
	}
}

void Rigidbody::Update()
{
	if (m_pBody)
	{
		PxTransform transform = m_pBody->getGlobalPose();
		GetTransform()->MarkDirty(*reinterpret_cast<Vector3*>(&transform.p), Vector3(1, 1, 1), *reinterpret_cast<Quaternion*>(&transform.q));
	}
}

void Rigidbody::SetType(const Type type)
{
	if (m_pBody)
	{
		FLUX_LOG(WARNING, "[Rigidbody::SetType] > Rigidbody already attached to the scene! Set type before adding it to the node.");
		return;
	}
	m_Type = type;
}
