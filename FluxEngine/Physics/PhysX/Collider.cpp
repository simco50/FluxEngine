#include "FluxEngine.h"
#include "Collider.h"
#include "Rigidbody.h"
#include "PhysicsSystem.h"

Collider::Collider(PhysicsSystem* pSystem) :
	m_pSystem(pSystem)
{

}

Collider::~Collider()
{
}

physx::PxShape* Collider::SetShape(const physx::PxGeometry& pGeometry, physx::PxMaterial* pMaterial /*= nullptr*/, physx::PxShapeFlags shapeFlags /*= physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE | physx::PxShapeFlag::eVISUALIZATION*/)
{
	if (m_pRigidbody == nullptr)
	{
		FLUX_LOG(WARNING, "[Collider::AddShape] Node does not have a Rigidbody");
		return nullptr;
	}

	if (pMaterial == nullptr)
		m_pMaterial = m_pSystem->GetDefaultMaterial();
	else
		m_pMaterial = pMaterial;

	PxShape* pShape = m_pSystem->GetPhysics()->createShape(pGeometry, &m_pMaterial, 1, false, shapeFlags);
	m_pRigidbody->GetBody()->attachShape(*pShape);
	m_pShape = pShape;
	return pShape;
}

void Collider::RemoveShape()
{
	if (m_pRigidbody == nullptr || m_pShape == nullptr)
		return;

	m_pRigidbody->GetBody()->detachShape(*m_pShape);
}

void Collider::SetTrigger(bool isTrigger)
{
	if (m_pShape == nullptr)
	{
		FLUX_LOG(WARNING, "[Collider::SetTrigger] Collider does not have a shape set");
		return;
	}
	m_pShape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, isTrigger);
}

void Collider::OnNodeSet(SceneNode* pNode)
{
	Component::OnNodeSet(pNode);

	m_pRigidbody = GetComponent<Rigidbody>();
	if (m_pRigidbody == nullptr)
	{
		FLUX_LOG(ERROR, "[Collider::OnSceneSet] > Object does not have a Rigidbody");
		return;
	}
}

void Collider::OnNodeRemoved()
{
	if (m_pRigidbody && m_pShape)
		m_pRigidbody->GetBody()->detachShape(*m_pShape);
}
