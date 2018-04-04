#pragma once
#include "Rendering/Drawable.h"

class Mesh;
class Skeleton;

class Model : public Drawable
{
	FLUX_OBJECT(Model, Drawable)

public:
	Model(Context* pContext);
	virtual ~Model();

	virtual void OnSceneSet(Scene* pScene) override;
	virtual void OnNodeSet(SceneNode* pNode) override;
	virtual void OnMarkedDirty(const Transform* pTransform) override;

	Matrix* GetBoneMatrices();
	const Skeleton& GetSkeleton() const;

	void SetMesh(Mesh* pMesh);
	void SetMaterial(Material* pMaterial);
	void SetMaterial(int index, Material* pMaterial);
private:
	Mesh* m_pMesh = nullptr;
	std::vector<Matrix> m_SkinMatrices;
	float m_AnimationTime = 0.0f;
};