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
	AddAnimation(pAnimation);

	if (m_pAnimation)
	{
		m_pModel->RemoveAnimationState(m_pModel->GetAnimationState(m_pAnimation->GetNameHash()));
	}

	m_pAnimation = pAnimation;
	pAnimation->ResolveBoneIndices(m_pModel->GetSkeleton());
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

void Animator::Play()
{
	if (m_pAnimation)
	{
		m_Playing = true;
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
	if (m_Playing)
	{
		AnimationState* pState = GetAnimationState(m_pAnimation->GetNameHash());
		if (pState)
		{
			pState->AddTime(GameTimer::DeltaTime());
		}
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

void Animator::AddAnimation(Animation* pAnimation)
{
	if (std::find(m_pAnimations.begin(), m_pAnimations.end(), pAnimation) == m_pAnimations.end())
	{
		m_pAnimations.push_back(pAnimation);
	}
}

void Animator::CreateUI()
{
	if (ImGui::Button(m_Playing ? "Pause" : "Play"))
	{
		if (m_Playing)
		{
			Stop();
		}
		else
		{
			Play();
		}
	}

	static int currentAnimation = 0;
	int current = currentAnimation;
	ImGui::Combo("Animation", &current, [](void* pData, int i, const char** pName)
	{
		Animator* pAnimator = (Animator*)pData;
		if (i < (int)pAnimator->m_pAnimations.size())
		{
			*pName = pAnimator->m_pAnimations[i]->GetName().c_str();
			return true;
		}
		return false;
	}, this, (int)m_pAnimations.size());
	if (currentAnimation != current)
	{
		Play(m_pAnimations[current]);
		currentAnimation = current;
	}

	if (m_pAnimation)
	{
		AnimationState* pState = GetAnimationState(m_pAnimation->GetNameHash());
		if (pState)
		{
			ImGui::LabelText("Time", "%f/%fs", pState->GetTime(), pState->GetDuration());
			bool looped = pState->IsLooped();
			ImGui::Checkbox("Loop", &looped);
			pState->SetLoop(looped);
		}
	}
}