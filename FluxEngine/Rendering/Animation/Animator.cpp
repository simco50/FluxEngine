#include "FluxEngine.h"
#include "Animator.h"
#include "Skeleton.h"
#include "Animation.h"
#include "AnimatedModel.h"
#include "Scenegraph\SceneNode.h"
#include "Rendering/Mesh.h"
#include "Scenegraph\Scene.h"
#include "AnimationState.h"

Animator::Animator(Context* pContext) :
	Component(pContext)
{
}

Animator::~Animator()
{

}

void Animator::Play()
{
	AnimatedModel* pModel = GetComponent<AnimatedModel>();
	if (pModel)
	{
		pModel->AddAnimationState(m_pModel->GetMesh()->GetAnimation(0));
	}
}

void Animator::Stop()
{
}

void Animator::Reset()
{
}

void Animator::Update()
{
	AnimationState* pState = GetAnimationState(0);
	if (pState)
	{
		pState->AddTime(GameTimer::DeltaTime());
	}
}

void Animator::OnSceneSet(Scene* pScene)
{
	Component::OnSceneSet(pScene);

	m_pModel = m_pNode->GetComponent<AnimatedModel>();
	pScene->OnSceneUpdate().AddRaw(this, &Animator::Update);
}

AnimationState * Animator::GetAnimationState(const int index)
{
	AnimatedModel* pModel = GetComponent<AnimatedModel>();
	if (pModel)
	{
		return pModel->GetAnimationState(index);
	}
	return nullptr;
}