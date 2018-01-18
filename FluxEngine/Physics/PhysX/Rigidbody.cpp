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

	CreateBody(m_Type);
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
	if (m_Type == type)
		return;
	CreateBody(type);
}

void Rigidbody::CreateBody(const Type type)
{
	m_Type = type;
	if (m_pNode == nullptr)
		return;

	Transform* pTransform = GetTransform();
	PxTransform transform(*reinterpret_cast<const PxVec3*>(&pTransform->GetWorldPosition()), *reinterpret_cast<const PxQuat*>(&pTransform->GetWorldRotation()));

	PxRigidActor* pNewBody;
	switch (m_Type)
	{
	case Rigidbody::Type::Dynamic:
		pNewBody = m_pSystem->GetPhysics()->createRigidDynamic(transform);
		break;
	case Rigidbody::Type::Static:
	default:
		pNewBody = m_pSystem->GetPhysics()->createRigidStatic(transform);
		break;
	}

	if (m_pBody)
	{
		FLUX_LOG(WARNING, "[Rigidbody::SetType] > Rigidbody already attached to the scene! Recreating body and transferring colliders. Set type before adding it to the node.");

		vector<PxShape*> shapes(m_pBody->getNbShapes());
		m_pBody->getShapes(shapes.data(), (PxU32)shapes.size(), 0);

		for (PxShape* pShape : shapes)
		{
			pNewBody->attachShape(*pShape);
			m_pBody->detachShape(*pShape);
		}
		if (m_pScene)
			m_pPhysicsScene->GetScene()->removeActor(*m_pBody);
	}
	m_pBody = pNewBody;
	m_pBody->userData = this;
	if (m_pScene)
		m_pPhysicsScene->GetScene()->addActor(*m_pBody);
}
