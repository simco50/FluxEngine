#pragma once
#include "Scenegraph\Component.h"

class AnimationState;
class AnimatedModel;
class Animation;

class Animator : public Component
{
	FLUX_OBJECT(Animator, Component)

public:
	Animator(Context* pContext);
	virtual ~Animator();

	void Play(Animation* pAnimation);
	void Stop();
	void Reset();

	void Update();

	virtual void OnSceneSet(Scene* pScene) override;

private:
	AnimationState * GetAnimationState(const StringHash hash);

	AnimatedModel* m_pModel = nullptr;
	Animation* m_pAnimation = nullptr;
	bool m_Playing = false;
};