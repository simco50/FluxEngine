#pragma once
#include "Rendering/Model.h"
#include "AnimationState.h"
#include "Math/DualQuaternion.h"

class Mesh;

class AnimatedModel : public Model
{
	FLUX_OBJECT(AnimatedModel, Drawable)

public:
	AnimatedModel(Context* pContext);
	virtual ~AnimatedModel();

	virtual void Update() override;
	virtual void SetMesh(Mesh* pMesh) override;

	Matrix* GetSkinMatrices() { return m_SkinMatrices.data(); }
	DualQuaternion* GetSkinQuaternions() { return m_SkinQuaternions.data(); }
	const Skeleton& GetSkeleton() const;

	AnimationState* AddAnimationState(Animation* pAnimation);
	void RemoveAnimationState(AnimationState* pAnimation);
	AnimationState* GetAnimationState(const StringHash hash);

private:
	std::vector<Matrix> m_SkinMatrices;
	std::vector<DualQuaternion> m_SkinQuaternions;
	std::vector<AnimationState> m_AnimationStates;
};