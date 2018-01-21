#pragma once

#include "SceneGraph/Component.h"

class PhysicsSystem;
class PhysicsScene;
class Rigidbody;

class Collider : public Component
{
	FLUX_OBJECT(Collider, Component)

public:
	Collider(PhysicsSystem* pSystem, PxMaterial* pMaterial, physx::PxShapeFlags shapeFlags);
	virtual ~Collider();

	void SetTrigger(bool isTrigger);
	void SetMaterial(PxMaterial* pMaterial);

	physx::PxShape* GetShape() const { return m_pShape; }

	virtual void OnNodeSet(SceneNode* pNode) override;
	virtual void OnNodeRemoved() override;
	virtual bool CanHaveMultiple() const override { return true; }

	PxGeometry* GetGeometry() const { return m_pGeometry; }

protected:
	void CreateShape();

	void RemoveShape();

	PhysicsSystem* m_pPhysicsSystem;
	PhysicsScene* m_pScene = nullptr;
	Rigidbody* m_pRigidbody = nullptr;
	PxGeometry* m_pGeometry;
	physx::PxMaterial* m_pMaterial = nullptr;
	physx::PxShape* m_pShape = nullptr;
	physx::PxShapeFlags m_ShapeFlags;
};

class SphereCollider : public Collider
{
	FLUX_OBJECT(SphereCollider, Collider)

public:
	SphereCollider(PhysicsSystem* pSystem, float radius, PxMaterial* pMaterial = nullptr, physx::PxShapeFlags shapeFlags = physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE | physx::PxShapeFlag::eVISUALIZATION);
	virtual ~SphereCollider();
};

class BoxCollider : public Collider
{
	FLUX_OBJECT(BoxCollider, Collider)

public:
	BoxCollider(PhysicsSystem* pSystem, const Vector3& extents, PxMaterial* pMaterial = nullptr, physx::PxShapeFlags shapeFlags = physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE | physx::PxShapeFlag::eVISUALIZATION);
	BoxCollider(PhysicsSystem* pSystem, const BoundingBox& boundingBox, PxMaterial* pMaterial = nullptr, physx::PxShapeFlags shapeFlags = physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE | physx::PxShapeFlag::eVISUALIZATION);
	
	virtual ~BoxCollider();
};

class PlaneCollider : public Collider
{
	FLUX_OBJECT(PlaneCollider, Collider)

public:
	PlaneCollider(PhysicsSystem* pSystem, PxMaterial* pMaterial = nullptr, physx::PxShapeFlags shapeFlags = physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE | physx::PxShapeFlag::eVISUALIZATION);
	virtual ~PlaneCollider();
};

class CapsuleCollider : public Collider
{
	FLUX_OBJECT(CapsuleCollider, Collider)

public:
	CapsuleCollider(PhysicsSystem* pSystem, const float radius, const float height, PxMaterial* pMaterial = nullptr, physx::PxShapeFlags shapeFlags = physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE | physx::PxShapeFlag::eVISUALIZATION);
	virtual ~CapsuleCollider();
};