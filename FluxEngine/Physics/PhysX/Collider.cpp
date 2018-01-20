#include "FluxEngine.h"
#include "Collider.h"
#include "Rigidbody.h"
#include "PhysicsSystem.h"
#include "Scenegraph\SceneNode.h"

Collider::Collider(PhysicsSystem* pPhysicsSystem) :
	m_pPhysicsSystem(pPhysicsSystem)
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

	AUTOPROFILE(Collider_SetShape);
	m_pMaterial = pMaterial ? pMaterial : m_pPhysicsSystem->GetDefaultMaterial();
	PxShape* pShape = m_pPhysicsSystem->GetPhysics()->createShape(pGeometry, &m_pMaterial, 1, true, shapeFlags);
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
		FLUX_LOG(WARNING, "[Collider::SetTrigger] Collider does not have a shape");
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
		m_pRigidbody = new Rigidbody(m_pPhysicsSystem);
		pNode->AddComponent(m_pRigidbody);
		return;
	}
}

void Collider::OnNodeRemoved()
{
	if (m_pRigidbody && m_pShape)
		m_pRigidbody->GetBody()->detachShape(*m_pShape);
}