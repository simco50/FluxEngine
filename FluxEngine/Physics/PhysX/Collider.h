#pragma once

#include "SceneGraph/Component.h"

class PhysicsSystem;
class PhysicsScene;
class Rigidbody;
class PhysicsMesh;

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
	virtual void CreateGeometry() = 0;
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
protected:
	virtual void CreateGeometry() override;
private:
	float m_Radius;
};

class BoxCollider : public Collider
{
	FLUX_OBJECT(BoxCollider, Collider)

public:
	BoxCollider(PhysicsSystem* pSystem, const Vector3& extents, PxMaterial* pMaterial = nullptr, physx::PxShapeFlags shapeFlags = physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE | physx::PxShapeFlag::eVISUALIZATION);
	BoxCollider(PhysicsSystem* pSystem, const BoundingBox& boundingBox, PxMaterial* pMaterial = nullptr, physx::PxShapeFlags shapeFlags = physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE | physx::PxShapeFlag::eVISUALIZATION);
	
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
	PlaneCollider(PhysicsSystem* pSystem, PxMaterial* pMaterial = nullptr, physx::PxShapeFlags shapeFlags = physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE | physx::PxShapeFlag::eVISUALIZATION);
	virtual ~PlaneCollider() {}
protected:
	virtual void CreateGeometry() override;
};

class CapsuleCollider : public Collider
{
	FLUX_OBJECT(CapsuleCollider, Collider)

public:
	CapsuleCollider(PhysicsSystem* pSystem, const float radius, const float height, PxMaterial* pMaterial = nullptr, physx::PxShapeFlags shapeFlags = physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE | physx::PxShapeFlag::eVISUALIZATION);
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
	MeshCollider(PhysicsSystem* pSystem, PhysicsMesh* pMesh, PxMaterial* pMaterial = nullptr, physx::PxShapeFlags shapeFlags = physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE | physx::PxShapeFlag::eVISUALIZATION);
	MeshCollider(PhysicsSystem* pSystem, const std::string& filePath, PxMaterial* pMaterial = nullptr, physx::PxShapeFlags shapeFlags = physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE | physx::PxShapeFlag::eVISUALIZATION);
	virtual ~MeshCollider() {}

protected:
	virtual void CreateGeometry() override;
private:
	PhysicsMesh * m_pMesh;
};