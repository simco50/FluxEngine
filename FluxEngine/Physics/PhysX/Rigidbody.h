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
	virtual void OnMarkedDirty(const Matrix& worldMatrix) override;
	virtual void Update() override;

	enum class Type
	{
		Static,
		Dynamic,
	};
	void SetType(const Rigidbody::Type type);

	physx::PxRigidBody* GetBody() const { return reinterpret_cast<physx::PxRigidBody*>(m_pBody); }

private:
	void CreateBody(const Rigidbody::Type type);

	PhysicsScene* m_pPhysicsScene = nullptr;
	PhysicsSystem* m_pSystem = nullptr;
	physx::PxRigidActor* m_pBody = nullptr;
	Rigidbody::Type m_Type = Type::Static;

	bool m_Dynamic = false;
};