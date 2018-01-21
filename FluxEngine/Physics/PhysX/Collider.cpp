#include "FluxEngine.h"
#include "Collider.h"
#include "Rigidbody.h"
#include "PhysicsSystem.h"
#include "Scenegraph\SceneNode.h"
#include "Rendering\Model.h"

Collider::Collider(PhysicsSystem* pPhysicsSystem, PxMaterial* pMaterial, physx::PxShapeFlags shapeFlags) :
	m_pPhysicsSystem(pPhysicsSystem),
	m_ShapeFlags(shapeFlags)
{
	m_pMaterial = pMaterial ? pMaterial : m_pPhysicsSystem->GetDefaultMaterial();
}

Collider::~Collider()
{
}

void Collider::CreateShape()
{
	m_pShape = m_pPhysicsSystem->GetPhysics()->createShape(*m_pGeometry, *m_pMaterial, true, m_ShapeFlags);
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

void Collider::SetMaterial(PxMaterial* pMaterial)
{
	if (m_pShape)
	{
		m_pMaterial = pMaterial;
		m_pShape->setMaterials(&pMaterial, 1);
	}
}

void Collider::OnNodeSet(SceneNode* pNode)
{
	Component::OnNodeSet(pNode);

	m_pRigidbody = GetComponent<Rigidbody>();
	if (m_pRigidbody == nullptr)
	{
		m_pRigidbody = new Rigidbody(m_pPhysicsSystem);
		pNode->AddComponent(m_pRigidbody);
	}
	CreateShape();
	if (m_pShape)
		m_pRigidbody->GetBody()->attachShape(*m_pShape);
}

void Collider::OnNodeRemoved()
{
	RemoveShape();
}

SphereCollider::SphereCollider(PhysicsSystem* pSystem,
	float radius,
	PxMaterial* pMaterial /*= nullptr*/,
	physx::PxShapeFlags shapeFlags /*= physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE | physx::PxShapeFlag::eVISUALIZATION*/) :
	Collider(pSystem, pMaterial, shapeFlags)
{
	m_pGeometry = new PxSphereGeometry(radius);
}

SphereCollider::~SphereCollider()
{
	SafeDelete(m_pGeometry);
}

BoxCollider::BoxCollider(PhysicsSystem* pSystem, const Vector3& extents, PxMaterial* pMaterial /*= nullptr*/, physx::PxShapeFlags shapeFlags /*= physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE | physx::PxShapeFlag::eVISUALIZATION*/) :
	Collider(pSystem, pMaterial, shapeFlags)
{
	m_pGeometry = new PxBoxGeometry(extents.x, extents.y, extents.z);
}

BoxCollider::BoxCollider(PhysicsSystem* pSystem, const BoundingBox& boundingBox, PxMaterial* pMaterial /*= nullptr*/, physx::PxShapeFlags shapeFlags /*= physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE | physx::PxShapeFlag::eVISUALIZATION*/):
	Collider(pSystem, pMaterial, shapeFlags)
{
	m_pGeometry = new PxBoxGeometry(boundingBox.Extents.x, boundingBox.Extents.y, boundingBox.Extents.z);
}

BoxCollider::~BoxCollider()
{
	SafeDelete(m_pGeometry);
}

PlaneCollider::PlaneCollider(PhysicsSystem* pSystem, PxMaterial* pMaterial /*= nullptr*/, physx::PxShapeFlags shapeFlags /*= physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE | physx::PxShapeFlag::eVISUALIZATION*/) :
	Collider(pSystem, pMaterial, shapeFlags)
{
	m_pGeometry = new PxPlaneGeometry();
}

PlaneCollider::~PlaneCollider()
{
	SafeDelete(m_pGeometry);
}

CapsuleCollider::CapsuleCollider(PhysicsSystem* pSystem, const float radius, const float height, PxMaterial* pMaterial /*= nullptr*/, physx::PxShapeFlags shapeFlags /*= physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE | physx::PxShapeFlag::eVISUALIZATION*/) :
	Collider(pSystem, pMaterial, shapeFlags)
{
	m_pGeometry = new PxCapsuleGeometry(radius, height / 2.0f);
}

CapsuleCollider::~CapsuleCollider()
{
	SafeDelete(m_pGeometry);
}
