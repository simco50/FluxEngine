#pragma once
#include "SceneGraph/Component.h"

class PhysicsSystem;
class Collider;
class Rigidbody;
class SceneNode;

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
	std::vector<Contact> Contacts;
	Collider* pCollider = nullptr;
	Rigidbody* pRigidbody = nullptr;
	SceneNode* pNode = nullptr;
};

class PhysicsScene : public Component, public physx::PxSimulationEventCallback
{
	FLUX_OBJECT(PhysicsScene, Component)

public:
	explicit PhysicsScene(Context* pContext);
	~PhysicsScene();

	virtual void OnSceneSet(Scene* pScene) override;
	virtual void OnSceneRemoved() override;

	void Update();

	physx::PxScene* GetScene() const { return m_pPhysicsScene; }

	bool Raycast(
		const Ray& ray,
		RaycastResult& outResult,
		const float length = FLT_MAX
	) const;

	void SetGravity(const float x, const float y, const float z);

	void AddRigidbody(Rigidbody* pRigidbody);
	void RemoveRigidbody(Rigidbody* pRigidbody);

	virtual void CreateUI() override;

private:
	static const float FIXED_TIME_STEP;

	virtual void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) override;
	virtual void onConstraintBreak(physx::PxConstraintInfo* /*constraints*/, physx::PxU32 /*count*/) override {}
	virtual void onWake(physx::PxActor** /*actors*/, physx::PxU32 /*count*/) override {}
	virtual void onSleep(physx::PxActor** /*actors*/, physx::PxU32 /*count*/) override {}
	virtual void onContact(const physx::PxContactPairHeader& /*pairHeader*/, const physx::PxContactPair* /*pairs*/, physx::PxU32 /*nbPairs*/) override;
	virtual void onAdvance(const physx::PxRigidBody*const* /*bodyBuffer*/, const physx::PxTransform* /*poseBuffer*/, const physx::PxU32 /*count*/) override {}

	float m_TimeAccumulator = 0.0;
	physx::PxScene* m_pPhysicsScene = nullptr;
	PhysicsSystem* m_pPhysicsSystem = nullptr;

	std::vector<Rigidbody*> m_Rigidbodies;
};