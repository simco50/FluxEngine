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
};

class Drawable : public Component
{
	FLUX_OBJECT(Drawable, Component)

public:
	Drawable() {}
	virtual ~Drawable() {}

	virtual void OnSceneSet(Scene* pScene) override;

	std::vector<Batch>& GetBatches() { return m_Batches; }

	const BoundingBox& GetBoundingBox() const { return m_BoundingBox; }
	virtual BoundingBox GetWorldBoundingBox() const { return m_BoundingBox; }

	bool DrawEnabled() const { return m_Draw; }

protected:
	bool m_Draw = true;
	std::vector<Batch> m_Batches;
	BoundingBox m_BoundingBox;
};