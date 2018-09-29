#include "FluxEngine.h"
#include "Collider.h"
#include "Rigidbody.h"
#include "PhysicsSystem.h"
#include "Scenegraph\SceneNode.h"
#include "Rendering\Model.h"
#include "PhysicsMesh.h"

using namespace physx;

Collider::Collider(Context* pContext, PxMaterial* pMaterial, physx::PxShapeFlags shapeFlags)
	: Component(pContext), m_ShapeFlags(shapeFlags), m_LocalPose(physx::PxTransform(PxIdentity))
{
	m_pPhysicsSystem = pContext->GetSubsystem<PhysicsSystem>();
	m_pMaterial = pMaterial ? pMaterial : m_pPhysicsSystem->GetDefaultMaterial();
}

Collider::~Collider()
{
	delete m_pGeometry;
}

void Collider::CreateShape()
{
	CreateGeometry();
	m_pShape = m_pPhysicsSystem->GetPhysics()->createShape(*m_pGeometry, *m_pMaterial, true, m_ShapeFlags);
	m_pShape->setLocalPose(m_LocalPose);
	m_pShape->userData = this;
	SetCollisionGroup(m_CollisionGroup, m_ListenForCollisionGroups);
}

void Collider::RemoveShape()
{
	if (m_pRigidbody == nullptr || m_pShape == nullptr)
	{
		return;
	}

	m_pRigidbody->GetBody()->detachShape(*m_pShape);
}

void Collider::SetTrigger(bool isTrigger)
{
	if (m_pShape == nullptr)
	{
		FLUX_LOG(Warning, "[Collider::SetTrigger] Collider does not have a shape");
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

void Collider::SetCollisionGroup(const CollisionGroup group, const CollisionGroup listenerForGroup)
{
	m_CollisionGroup = group;
	m_ListenForCollisionGroups = listenerForGroup;
	if (m_pShape)
	{
		PxFilterData filter;
		filter.setToDefault();
		filter.word0 = (unsigned int)group;
		filter.word1 = (unsigned int)(listenerForGroup | group);
		m_pShape->setSimulationFilterData(filter);
	}
}

void Collider::SetCollisionGroup(const CollisionGroup group)
{
	SetCollisionGroup(group, group);
}

void Collider::OnNodeSet(SceneNode* pNode)
{
	Component::OnNodeSet(pNode);

	m_pRigidbody = GetComponent<Rigidbody>();
	if (m_pRigidbody == nullptr)
	{
		m_pRigidbody = m_pNode->CreateComponent<Rigidbody>();
	}
	CreateShape();
	if (m_pShape)
	{
		m_pRigidbody->GetBody()->attachShape(*m_pShape);
	}
}

void Collider::OnNodeRemoved()
{
	RemoveShape();
}

SphereCollider::SphereCollider(Context* pContext, float radius, PxMaterial* pMaterial /*= nullptr*/, physx::PxShapeFlags shapeFlags /*= physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE | physx::PxShapeFlag::eVISUALIZATION*/)
	: Collider(pContext, pMaterial, shapeFlags),
	m_Radius(radius)
{}

SphereCollider::SphereCollider(Context* pContext, physx::PxMaterial* pMaterial /*= nullptr*/, physx::PxShapeFlags shapeFlags /*= physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE | physx::PxShapeFlag::eVISUALIZATION*/)
	: Collider(pContext, pMaterial, shapeFlags),
	m_Radius(0)
{

}

SphereCollider::~SphereCollider()
{
	delete m_pGeometry;
}

void SphereCollider::CreateGeometry()
{
	if (m_Radius <= 0.0f)
	{
		Drawable* pDrawable = GetComponent<Drawable>();
		if (pDrawable)
		{
			const BoundingBox& bb = pDrawable->GetBoundingBox();
			m_Radius = Math::Average3(bb.Extents.x, bb.Extents.y, bb.Extents.z);
			m_LocalPose = PxTransform(PxVec3(bb.Center.x, bb.Center.y, bb.Center.z), PxQuat(PxIdentity));
		}
	}
	m_pGeometry = new PxSphereGeometry(m_Radius);
}

//BOX
BoxCollider::BoxCollider(Context* pContext, const Vector3& extents, PxMaterial* pMaterial /*= nullptr*/, physx::PxShapeFlags shapeFlags /*= physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE | physx::PxShapeFlag::eVISUALIZATION*/) :
	Collider(pContext, pMaterial, shapeFlags),
	m_Extents(extents)
{}

BoxCollider::BoxCollider(Context* pContext, const BoundingBox& boundingBox, PxMaterial* pMaterial /*= nullptr*/, physx::PxShapeFlags shapeFlags /*= physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE | physx::PxShapeFlag::eVISUALIZATION*/):
	Collider(pContext, pMaterial, shapeFlags),
	m_Extents(boundingBox.Extents.x, boundingBox.Extents.y, boundingBox.Extents.z)
{
	m_LocalPose = PxTransform(PxVec3(boundingBox.Center.x, boundingBox.Center.y, boundingBox.Center.z), PxQuat(PxIdentity));
}

BoxCollider::BoxCollider(Context* pContext, physx::PxMaterial* pMaterial /*= nullptr*/, physx::PxShapeFlags shapeFlags /*= physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE | physx::PxShapeFlag::eVISUALIZATION*/) :
	Collider(pContext, pMaterial, shapeFlags)
{

}

void BoxCollider::CreateGeometry()
{
	if (m_Extents.LengthSquared() == 0)
	{
		Drawable* pDrawable = GetComponent<Drawable>();
		if (pDrawable)
		{
			const BoundingBox& bb = pDrawable->GetBoundingBox();
			m_Extents = Vector3(bb.Extents.x, bb.Extents.y, bb.Extents.z);
			m_LocalPose = PxTransform(PxVec3(bb.Center.x, bb.Center.y, bb.Center.z), PxQuat(PxIdentity));
		}
	}
	m_pGeometry = new PxBoxGeometry(m_Extents.x, m_Extents.y, m_Extents.z);
}

//PLANE
PlaneCollider::PlaneCollider(Context* pContext, PxMaterial* pMaterial /*= nullptr*/, physx::PxShapeFlags shapeFlags /*= physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE | physx::PxShapeFlag::eVISUALIZATION*/)
	: Collider(pContext, pMaterial, shapeFlags)
{}

void PlaneCollider::CreateGeometry()
{
	m_pGeometry = new PxPlaneGeometry();
	m_LocalPose = PxTransform(PxVec3(0, 0, 0), PxQuat(XM_PIDIV2, PxVec3(0, 0, 1)));
}

//CAPSULE
CapsuleCollider::CapsuleCollider(Context* pContext, const float radius, const float height, PxMaterial* pMaterial /*= nullptr*/, physx::PxShapeFlags shapeFlags /*= physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE | physx::PxShapeFlag::eVISUALIZATION*/)
	: Collider(pContext, pMaterial, shapeFlags),
	m_Radius(radius),
	m_Height(height / 2)
{}

CapsuleCollider::CapsuleCollider(Context* pContext, physx::PxMaterial* pMaterial /*= nullptr*/, physx::PxShapeFlags shapeFlags /*= physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE | physx::PxShapeFlag::eVISUALIZATION*/) :
	Collider(pContext, pMaterial, shapeFlags),
	m_Radius(0),
	m_Height(0)
{

}

void CapsuleCollider::CreateGeometry()
{
	if (m_Height == 0 || m_Radius == 0)
	{
		Drawable* pDrawable = GetComponent<Drawable>();
		if (pDrawable)
		{
			const BoundingBox& bb = pDrawable->GetBoundingBox();
			m_Radius = Math::Average(bb.Extents.x, bb.Extents.z);
			m_Height = bb.Extents.y / 2;
			m_LocalPose = PxTransform(PxVec3(bb.Center.x, bb.Center.y, bb.Center.z), PxQuat(XM_PIDIV2, PxVec3(0, 0, 1)));
		}
	}
	m_pGeometry = new PxCapsuleGeometry(m_Radius, m_Height);
}

//MESH
MeshCollider::MeshCollider(Context* pContext, PhysicsMesh* pMesh, PxMaterial* pMaterial /*= nullptr*/, physx::PxShapeFlags shapeFlags /*= physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE | physx::PxShapeFlag::eVISUALIZATION*/)
	: Collider(pContext, pMaterial, shapeFlags),
	m_pMesh(pMesh)
{}

MeshCollider::MeshCollider(Context* pContext, const std::string& filePath, PxMaterial* pMaterial /*= nullptr*/, physx::PxShapeFlags shapeFlags /*= physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE | physx::PxShapeFlag::eVISUALIZATION*/)
	: Collider(pContext, pMaterial, shapeFlags)
{
	m_pMesh = GetSubsystem<ResourceManager>()->Load<PhysicsMesh>(filePath);
}

void MeshCollider::CreateGeometry()
{
	if (m_pMesh == nullptr)
	{
		FLUX_LOG(Error, "[MeshCollider::MeshCollider] > Given PhysicsMesh is null");
		return;
	}
	switch (m_pMesh->GetMeshType())
	{
	case PhysicsMesh::Type::ConvexMesh:
		m_pGeometry = new PxConvexMeshGeometry((PxConvexMesh*)m_pMesh->GetMesh());
		break;
	case PhysicsMesh::Type::TriangleMesh:
		m_pGeometry = new PxTriangleMeshGeometry((PxTriangleMesh*)m_pMesh->GetMesh());
		break;
	case PhysicsMesh::Type::None:
	default:
		FLUX_LOG(Error, "[MeshCollider::MeshCollider] > Given PhysicsMesh has an invalid type");
		return;
	}
}