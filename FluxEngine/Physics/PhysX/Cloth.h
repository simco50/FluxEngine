#pragma once
#include "Scenegraph\Component.h"

class PhysicsSystem;
class PhysicsScene;
class Geometry;

class Cloth : public Component
{
public:
	Cloth(Context* pContext);
	virtual ~Cloth();

	virtual void OnNodeSet(SceneNode* pNode) override;
	virtual void OnNodeRemoved() override;
	virtual void OnSceneSet(Scene* pScene) override;

	void SetGeometry(Geometry* pGeometry);

private:
	void CreateCloth();

	physx::PxCloth* m_pCloth = nullptr;
	PhysicsSystem* m_pPhysicsSystem;
	PhysicsScene* m_pPhysicsScene = nullptr;
	Geometry* m_pGeometry = nullptr;
};