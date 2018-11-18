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

void Animator::Play(Animation* pAnimation)
{
	pAnimation->ResolveBoneIndices(m_pModel->GetSkeleton());
	m_pAnimation = pAnimation;
	AnimatedModel* pModel = GetComponent<AnimatedModel>();
	if (pModel)
	{
		AnimationState* pState = pModel->GetAnimationState(m_pAnimation->GetNameHash());
		if (pState == nullptr)
		{
			pModel->AddAnimationState(m_pAnimation);
			m_Playing = true;
		}
	}
}

void Animator::Stop()
{
	m_Playing = false;
}

void Animator::Reset()
{
}

void Animator::Update()
{
	AnimationState* pState = GetAnimationState(m_pAnimation->GetNameHash());
	if (pState && m_Playing)
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

AnimationState * Animator::GetAnimationState(const StringHash hash)
{
	AnimatedModel* pModel = GetComponent<AnimatedModel>();
	if (pModel)
	{
		return pModel->GetAnimationState(hash);
	}
	return nullptr;
}