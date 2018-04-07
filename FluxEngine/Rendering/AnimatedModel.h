#pragma once
#include "Model.h"

class Mesh;
class Skeleton;

class AnimatedModel : public Model
{
	FLUX_OBJECT(AnimatedModel, Drawable)

public:
	AnimatedModel(Context* pContext);
	virtual ~AnimatedModel();

	virtual void OnNodeSet(SceneNode* pNode) override;

	virtual void Update() override;

	void ApplyAnimation();
	const Matrix* GetSkinMatrices() const { return m_SkinMatrices.data(); }
	const Skeleton& GetSkeleton() const;

	void SetMesh(Mesh* pMesh);
private:
	Mesh* m_pMesh = nullptr;
	std::vector<Matrix> m_SkinMatrices;
	float m_AnimationTime = 0.0f;
};