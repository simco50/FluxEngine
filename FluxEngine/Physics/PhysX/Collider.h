#pragma once

#include "SceneGraph/Component.h"

class PhysicsSystem;
class PhysicsScene;
class Rigidbody;

class Collider : public Component
{
	FLUX_OBJECT(Collider, Component)

public:
	Collider(PhysicsSystem* pSystem);
	virtual ~Collider();

	physx::PxShape* SetShape(
		const physx::PxGeometry& pGeometry, 
		physx::PxMaterial* pMaterial = nullptr, 
		physx::PxShapeFlags shapeFlags = physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE | physx::PxShapeFlag::eVISUALIZATION);
	
	void SetTrigger(bool isTrigger);

	physx::PxShape* GetShape() const { return m_pShape; }

	virtual void OnNodeSet(SceneNode* pNode) override;
	virtual void OnNodeRemoved() override;

	virtual bool CanHaveMultiple() const override { return true; }

private:
	void RemoveShape();

	PhysicsSystem* m_pSystem;
	PhysicsScene* m_pScene = nullptr;
	Rigidbody* m_pRigidbody = nullptr;

	physx::PxMaterial* m_pMaterial = nullptr;
	physx::PxShape* m_pShape = nullptr;
};