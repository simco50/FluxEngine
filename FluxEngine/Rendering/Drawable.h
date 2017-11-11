#pragma once
#include "Scenegraph/Component.h"

class Scene;
class SceneNode;
class Material;
class Geometry;

struct Batch
{
	Material* pMaterial = nullptr;
	Geometry* pGeometry = nullptr;
	const XMFLOAT4X4* pModelMatrix = nullptr;
};

class Drawable : public Component
{
public:
	Drawable() {}
	virtual ~Drawable() {}

	virtual void OnSceneSet(Scene* pScene) override;

	std::vector<Batch>& GetBatches() { return m_Batches; }

protected:
	std::vector<Batch> m_Batches;
};