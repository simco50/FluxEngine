#pragma once

#include "Scenegraph/Component.h"

class PhysicsScene;
class PhysicsSystem;
class Collider;

struct CollisionResult;

DECLARE_MULTICAST_DELEGATE(OnTriggerDelegate, Collider*);
DECLARE_MULTICAST_DELEGATE(OnColliderDelegate, const CollisionResult&);

class Rigidbody : public Component
{
	FLUX_OBJECT(Rigidbody, Component)

public:
	explicit Rigidbody(Context* pContext);
	virtual ~Rigidbody();

	virtual void OnSceneSet(Scene* pScene) override;
	virtual void OnSceneRemoved() override;
	virtual void OnNodeSet(SceneNode* pNode) override;
	virtual void OnNodeRemoved() override;
	virtual void OnMarkedDirty(const SceneNode* pNode) override;
	void UpdateBody();

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
		if (m_Type == Type::Dynamic)
		{
			return reinterpret_cast<physx::PxRigidDynamic*>(m_pBody);
		}
		return nullptr;
	}
	template<>
	physx::PxRigidStatic* GetBody() const
	{
		if (m_Type == Type::Static)
		{
			return reinterpret_cast<physx::PxRigidStatic*>(m_pBody);
		}
		return nullptr;
	}

	OnTriggerDelegate& OnTriggerEnter() { return m_OnTriggerEnterEvent; }
	OnTriggerDelegate& OnTriggerExit() { return m_OnTriggerExitEvent; }
	OnColliderDelegate& OnCollisionEnter() { return m_OnCollisionEnterEvent; }
	OnColliderDelegate& OnCollisionExit() { return m_OnCollisionExitEvent; }

private:
	void CreateBody(const Rigidbody::Type type);

	PhysicsScene* m_pPhysicsScene = nullptr;
	PhysicsSystem* m_pPhysicsSystem = nullptr;

	bool m_Dynamic = false;
	Constraint m_Constraints = Constraint::None;
	Rigidbody::Type m_Type = Type::Static;

	physx::PxD6Joint* m_pConstraintJoint = nullptr;
	physx::PxRigidActor* m_pBody = nullptr;

	OnTriggerDelegate m_OnTriggerEnterEvent;
	OnTriggerDelegate m_OnTriggerExitEvent;
	OnColliderDelegate m_OnCollisionEnterEvent;
	OnColliderDelegate m_OnCollisionExitEvent;
};