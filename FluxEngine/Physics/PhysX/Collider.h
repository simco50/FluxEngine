#pragma once

#include "SceneGraph/Component.h"

class PhysicsSystem;
class PhysicsScene;
class Rigidbody;
class PhysicsMesh;

enum class CollisionGroup : unsigned int
{
	None = 0,
	Group0 = 1 << 0,
	Group1 = 1 << 1,
	Group2 = 1 << 2,
	Group3 = 1 << 3,
	Group4 = 1 << 4,
};
inline CollisionGroup operator|(const CollisionGroup a, const CollisionGroup b) { return static_cast<CollisionGroup>(static_cast<unsigned int>(a) | static_cast<unsigned int>(b)); }
inline CollisionGroup& operator|=(CollisionGroup& a, const CollisionGroup b) { a = static_cast<CollisionGroup>(static_cast<unsigned int>(a) | static_cast<unsigned int>(b)); return a; }

class Collider : public Component
{
	FLUX_OBJECT_ABSTRACT(Collider, Component)

public:
	Collider(Context* pContext, physx::PxMaterial* pMaterial, physx::PxShapeFlags shapeFlags);
	virtual ~Collider();

	void SetTrigger(bool isTrigger);
	void SetMaterial(physx::PxMaterial* pMaterial);
	void SetCollisionGroup(const CollisionGroup group, const CollisionGroup listenerForGroup);
	void SetCollisionGroup(const CollisionGroup group);

	physx::PxShape* GetShape() const { return m_pShape; }

	virtual void OnNodeSet(SceneNode* pNode) override;
	virtual void OnNodeRemoved() override;
	virtual bool CanHaveMultiple() const override { return true; }

	physx::PxGeometry* GetGeometry() const { return m_pGeometry; }

protected:
	virtual void CreateGeometry() = 0;
	void CreateShape();
	void RemoveShape();

	PhysicsSystem* m_pPhysicsSystem;
	PhysicsScene* m_pScene = nullptr;
	Rigidbody* m_pRigidbody = nullptr;
	physx::PxGeometry* m_pGeometry = nullptr;
	CollisionGroup m_CollisionGroup = CollisionGroup::Group0;
	CollisionGroup m_ListenForCollisionGroups = CollisionGroup::Group0;
	physx::PxMaterial* m_pMaterial = nullptr;
	physx::PxShape* m_pShape = nullptr;
	physx::PxShapeFlags m_ShapeFlags;
	physx::PxTransform m_LocalPose;
};

class SphereCollider : public Collider
{
	FLUX_OBJECT(SphereCollider, Collider)

public:
	SphereCollider(Context* pContext, float radius, physx::PxMaterial* pMaterial = nullptr, physx::PxShapeFlags shapeFlags = physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE | physx::PxShapeFlag::eVISUALIZATION);
	SphereCollider(Context* pContext, physx::PxMaterial* pMaterial = nullptr, physx::PxShapeFlags shapeFlags = physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE | physx::PxShapeFlag::eVISUALIZATION);
	virtual ~SphereCollider();
protected:
	virtual void CreateGeometry() override;
private:
	float m_Radius;
};

class BoxCollider : public Collider
{
	FLUX_OBJECT(BoxCollider, Collider)

public:
	BoxCollider(Context* pContext, const Vector3& extents, physx::PxMaterial* pMaterial = nullptr, physx::PxShapeFlags shapeFlags = physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE | physx::PxShapeFlag::eVISUALIZATION);
	BoxCollider(Context* pContext, const BoundingBox& boundingBox, physx::PxMaterial* pMaterial = nullptr, physx::PxShapeFlags shapeFlags = physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE | physx::PxShapeFlag::eVISUALIZATION);
	BoxCollider(Context* pContext, physx::PxMaterial* pMaterial = nullptr, physx::PxShapeFlags shapeFlags = physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE | physx::PxShapeFlag::eVISUALIZATION);
	
	virtual ~BoxCollider() {}
protected:
	virtual void CreateGeometry() override;
private:
	Vector3 m_Extents;
};

class PlaneCollider : public Collider
{
	FLUX_OBJECT(PlaneCollider, Collider)

public:
	PlaneCollider(Context* pContext, physx::PxMaterial* pMaterial = nullptr, physx::PxShapeFlags shapeFlags = physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE | physx::PxShapeFlag::eVISUALIZATION);
	virtual ~PlaneCollider() {}
protected:
	virtual void CreateGeometry() override;
};

class CapsuleCollider : public Collider
{
	FLUX_OBJECT(CapsuleCollider, Collider)

public:
	CapsuleCollider(Context* pContext, const float radius, const float height, physx::PxMaterial* pMaterial = nullptr, physx::PxShapeFlags shapeFlags = physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE | physx::PxShapeFlag::eVISUALIZATION);
	CapsuleCollider(Context* pContext, physx::PxMaterial* pMaterial = nullptr, physx::PxShapeFlags shapeFlags = physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE | physx::PxShapeFlag::eVISUALIZATION);
	virtual ~CapsuleCollider() {}
protected:
	virtual void CreateGeometry() override;
private:
	float m_Radius;
	float m_Height;
};

class MeshCollider : public Collider
{
	FLUX_OBJECT(MeshCollider, Collider)

public:
	MeshCollider(Context* pContext, PhysicsMesh* pMesh, physx::PxMaterial* pMaterial = nullptr, physx::PxShapeFlags shapeFlags = physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE | physx::PxShapeFlag::eVISUALIZATION);
	MeshCollider(Context* pContext, const std::string& filePath, physx::PxMaterial* pMaterial = nullptr, physx::PxShapeFlags shapeFlags = physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE | physx::PxShapeFlag::eVISUALIZATION);
	virtual ~MeshCollider() {}

protected:
	virtual void CreateGeometry() override;
private:
	PhysicsMesh * m_pMesh;
};