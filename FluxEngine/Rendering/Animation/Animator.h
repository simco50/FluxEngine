#pragma once
#include "Scenegraph\Component.h"

struct Bone;
struct AnimationNode;
class Animation;
class AnimatedModel;

struct AnimationKeyState
{
	const Bone* pBone = nullptr;
	int KeyFrame = 0;
	const AnimationNode* pNode = nullptr;

	void GetFrameIndex(float time, int& index) const;
	void GetMatrix(const float time, Matrix& matrix);
};

struct AnimationState
{
	AnimationState(Animation* pAnimation, AnimatedModel* pModel);

	Animation* pAnimation = nullptr;
	AnimatedModel* pModel = nullptr;
	Bone* pRootBone = nullptr;
	std::vector<AnimationKeyState> KeyStates;

	float Time = 0.0f;

	void AddTime(const float time) { Time += time; }

	void Apply();
	void CalculateAnimations(Bone* pBone, Matrix parentMatrix);
};

class Animator : public Component
{
	FLUX_OBJECT(Animator, Component)

public:
	Animator(Context* pContext);
	virtual ~Animator();

	void Play();
	void Stop();
	void Reset();

	void Update();

	virtual void OnSceneSet(Scene* pScene) override;

private:
	AnimationState * GetAnimationState(const int index);

	AnimatedModel* m_pModel = nullptr;
};