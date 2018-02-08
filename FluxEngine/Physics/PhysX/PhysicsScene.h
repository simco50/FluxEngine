#pragma once
#include "SceneGraph/Component.h"

class PhysicsSystem;
class Collider;
class Rigidbody;
class SceneNode;
class Transform;

struct RaycastResult
{
	bool Hit = false;
	Collider* pCollider = nullptr;
	Rigidbody* pRigidbody = nullptr;
	float Distance = -1.0f;
	Vector3 Normal = Vector3();
	Vector3 Position = Vector3();
};

struct CollisionResult
{
	bool HasContacts() const { return Contacts.size() > 0; }
	struct Contact
	{
		Vector3 Position = Vector3();
		Vector3 Normal = Vector3();
	};
	vector<Contact> Contacts;
	Collider* pCollider = nullptr;
	Rigidbody* pRigidbody = nullptr;
	SceneNode* pNode = nullptr;
	Transform* pTransform = nullptr;
};

class PhysicsScene : public Component, public PxSimulationEventCallback
{
	FLUX_OBJECT(PhysicsScene, Component)

public:
	PhysicsScene(PhysicsSystem* pSystem);
	~PhysicsScene();

	virtual void OnSceneSet(Scene* pScene) override;
	virtual void OnSceneRemoved() override;
	virtual void Update() override;

	physx::PxScene* GetScene() const { return m_pPhysicsScene; }

	bool Raycast(
		const Vector3& origin,
		const Vector3& direction,
		RaycastResult& outResult,
		const float length = numeric_limits<float>::max()
	) const;

	void SetGravity(const float x, const float y, const float z);

private:
	virtual void onTrigger(PxTriggerPair* pairs, PxU32 count) override;
	virtual void onConstraintBreak(PxConstraintInfo* /*constraints*/, PxU32 /*count*/) override {}
	virtual void onWake(PxActor** /*actors*/, PxU32 /*count*/) override {}
	virtual void onSleep(PxActor** /*actors*/, PxU32 /*count*/) override {}
	virtual void onContact(const PxContactPairHeader& /*pairHeader*/, const PxContactPair* /*pairs*/, PxU32 /*nbPairs*/) override;
	virtual void onAdvance(const PxRigidBody*const* /*bodyBuffer*/, const PxTransform* /*poseBuffer*/, const PxU32 /*count*/) override {}

	physx::PxScene* m_pPhysicsScene = nullptr;
	PhysicsSystem* m_pPhysicsSystem = nullptr;
};