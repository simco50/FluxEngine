#pragma once

#include "Scenegraph/Component.h"

class PhysicsScene;
class PhysicsSystem;

class Rigidbody : public Component
{
	FLUX_OBJECT(Rigidbody, Component)

public:
	Rigidbody(PhysicsSystem* pSystem);
	~Rigidbody();

	virtual void OnSceneSet(Scene* pScene) override;
	virtual void OnSceneRemoved() override;
	virtual void OnNodeSet(SceneNode* pNode) override;
	virtual void OnNodeRemoved() override;

	physx::PxRigidBody* GetBody() const { return reinterpret_cast<physx::PxRigidBody*>(m_pBody); }

private:
	PhysicsScene* m_pPhysicsScene = nullptr;
	PhysicsSystem* m_pSystem = nullptr;
	physx::PxRigidActor* m_pBody = nullptr;

	bool m_Dynamic = false;
};