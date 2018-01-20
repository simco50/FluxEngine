#pragma once

#include "Scenegraph/Component.h"

class PhysicsScene;
class PhysicsSystem;

class Rigidbody : public Component
{
	FLUX_OBJECT(Rigidbody, Component)

public:
	Rigidbody(PhysicsSystem* pSystem);
	virtual ~Rigidbody();

	virtual void OnSceneSet(Scene* pScene) override;
	virtual void OnSceneRemoved() override;
	virtual void OnNodeSet(SceneNode* pNode) override;
	virtual void OnNodeRemoved() override;
	virtual void OnMarkedDirty(const Matrix& worldMatrix) override;
	virtual void Update() override;

	void SetKinematic(const bool isKinematic);

	enum Type
	{
		Static,
		Dynamic,
	};
	void SetBodyType(const Rigidbody::Type type);
	Rigidbody::Type GetBodyType() const { return m_Type; }

	enum Constraint
	{
		None = 0,
		XRotation = 1 << 0,
		YRotation = 1 << 1,
		ZRotation = 1 << 2,
		XPosition = 1 << 3,
		YPosition = 1 << 4,
		ZPosition = 1 << 5
	};
	void SetConstraints(const Constraint constraints);
	Rigidbody::Constraint GetConstraints() const { return m_Constraints; }

	physx::PxRigidActor* GetBody() const { return m_pBody; }
	template<typename T>
	T* GetBody() const;
	template<>
	physx::PxRigidDynamic* GetBody() const 
	{ 
		checkf(m_Type == Type::Dynamic, "[Rigidbody::GetBody()] Rigidbody is not Dynamic");
		return reinterpret_cast<physx::PxRigidDynamic*>(m_pBody); 
	}
	template<>
	physx::PxRigidStatic* GetBody() const 
	{
		checkf(m_Type == Type::Static, "[Rigidbody::GetBody()] Rigidbody is not Static");
		return reinterpret_cast<physx::PxRigidStatic*>(m_pBody);
	}

	MulticastDelegate<Rigidbody*, bool>& OnTrigger() { return m_OnTriggerEvent; }

private:
	void CreateBody(const Rigidbody::Type type);

	PhysicsScene* m_pPhysicsScene = nullptr;
	PhysicsSystem* m_pPhysicsSystem = nullptr;

	bool m_Dynamic = false;
	Constraint m_Constraints = Constraint::None;
	Rigidbody::Type m_Type = Type::Static;

	PxD6Joint* m_pConstraintJoint = nullptr;
	physx::PxRigidActor* m_pBody = nullptr;

	MulticastDelegate<Rigidbody*, bool> m_OnTriggerEvent;
};