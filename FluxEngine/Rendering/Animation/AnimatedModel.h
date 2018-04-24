#pragma once
#include "Rendering/Model.h"
#include "AnimationState.h"

class Mesh;

class AnimatedModel : public Model
{
	FLUX_OBJECT(AnimatedModel, Drawable)

public:
	AnimatedModel(Context* pContext);
	virtual ~AnimatedModel();

	virtual void OnNodeSet(SceneNode* pNode) override;
	virtual void Update() override;

	Matrix* GetSkinMatrices() { return m_SkinMatrices.data(); }
	const Skeleton& GetSkeleton() const;

	AnimationState* AddAnimationState(Animation* pAnimation);
	void RemoveAnimationState(AnimationState* pAnimation);
	AnimationState* GetAnimationState(const StringHash hash);

private:
	std::vector<Matrix> m_SkinMatrices;
	std::vector<AnimationState> m_AnimationStates;
};