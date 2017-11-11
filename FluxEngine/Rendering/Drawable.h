#pragma once
#include "Scenegraph/Component.h"

class Scene;
class SceneNode;
class Material;
class Geometry;

struct Batch
{
	Material* pMaterial;
	Geometry* pGeometry;
	XMFLOAT3X3* pModelMatrix;
};

class Drawable : public Component
{
public:
	Drawable() {}
	~Drawable() {};

	virtual void OnSceneSet(Scene* pScene) override;

	std::vector<Batch>& GetBatches() { return m_Batches; }

protected:
	std::vector<Batch> m_Batches;
};