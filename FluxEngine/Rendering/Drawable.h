#pragma once
#include "Scenegraph/Component.h"

class Scene;
class SceneNode;
class Material;
class Geometry;

struct Batch
{
	const Material* pMaterial = nullptr;
	const Geometry* pGeometry = nullptr;
	const Matrix* pModelMatrix = nullptr;
	const Matrix* pSkinMatrices = nullptr;
	int NumSkinMatrices = 1;
};

class Drawable : public Component
{
	FLUX_OBJECT(Drawable, Component)

public:
	Drawable(Context* pContext) : Component(pContext) {}
	virtual ~Drawable() {}
	virtual void Update() {}

	virtual void OnSceneSet(Scene* pScene) override;

	std::vector<Batch>& GetBatches() { return m_Batches; }

	const BoundingBox& GetBoundingBox() const { return m_BoundingBox; }
	BoundingBox GetWorldBoundingBox() const;

	bool DrawEnabled() const { return m_Draw; }

protected:
	bool m_Draw = true;
	std::vector<Batch> m_Batches;
	BoundingBox m_BoundingBox;
};