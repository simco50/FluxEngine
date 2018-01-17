#pragma once
#include "SceneGraph/Component.h"

class PhysicsSystem;

class PhysicsScene : public Component
{
public:
	PhysicsScene(PhysicsSystem* pSystem);
	~PhysicsScene();

	virtual void OnSceneSet(Scene* pScene) override;
	virtual void OnSceneRemoved() override;
	virtual void Update() override;

	physx::PxScene* GetScene() const { return m_pScene; }

private:
	physx::PxScene* m_pScene = nullptr;
	PhysicsSystem* m_pSystem = nullptr;
};