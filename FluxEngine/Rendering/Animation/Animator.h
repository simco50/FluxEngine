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

	void Play();
	void Play(Animation* pAnimation);
	void Stop();
	void Reset();

	void Update();

	virtual void OnSceneSet(Scene* pScene) override;
	virtual void CreateUI() override;

private:
	void AddAnimation(Animation* pAnimation);
	AnimationState * GetAnimationState(const StringHash hash);

	std::vector<Animation*> m_pAnimations;

	AnimatedModel* m_pModel = nullptr;
	Animation* m_pAnimation = nullptr;
	bool m_Playing = false;
};