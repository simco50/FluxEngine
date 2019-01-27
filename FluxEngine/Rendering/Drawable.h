#pragma once
#include "Scenegraph/Component.h"

class Scene;
class SceneNode;
class Material;
class Geometry;
struct DualQuaternion;

struct Batch
{
	const Material* pMaterial = nullptr;
	const Geometry* pGeometry = nullptr;
	const Matrix* pWorldMatrices = nullptr;
	const DualQuaternion* pSkinDualQuaternions = nullptr;
	int NumSkinMatrices = 1;
};

class Drawable : public Component
{
	FLUX_OBJECT_ABSTRACT(Drawable, Component)

public:
	explicit Drawable(Context* pContext) 
		: Component(pContext) {}
	virtual ~Drawable() {}
	virtual void Update() {}

	virtual void OnSceneSet(Scene* pScene) override;

	std::vector<Batch>& GetBatches() { return m_Batches; }

	void SetCullingEnabled(const bool enabled) { m_Cull = enabled; }
	bool GetCullingEnabled() const { return m_Cull; }
	const BoundingBox& GetBoundingBox() const { return m_BoundingBox; }
	BoundingBox GetWorldBoundingBox() const;

	virtual void CreateUI() override;

protected:
	bool m_Cull = true;
	std::vector<Batch> m_Batches;
	BoundingBox m_BoundingBox;
};