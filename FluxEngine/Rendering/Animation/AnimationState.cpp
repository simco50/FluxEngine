#include "FluxEngine.h"
#include "AnimationState.h"
#include "Animation.h"
#include "AnimatedModel.h"
#include "Skeleton.h"
#include "Scenegraph/SceneNode.h"

void AnimationKeyState::GetFrameIndex(float time, int& index) const
{
	if (time < 0.0f)
	{
		time = 0.0f;
	}
	if (index >= (int)pNode->Keys.size())
	{
		index = (int)pNode->Keys.size() - 1;
	}

	while (index && time < pNode->Keys[index].Time)
	{
		--index;
	}
	while (index < (int)pNode->Keys.size() - 1 && time >= pNode->Keys[index + 1].Time)
	{
		++index;
	}
}

void AnimationKeyState::GetTransform(float time, Vector3& scale, Quaternion& rotation, Vector3& translation)
{
	if (pNode->Keys.size() == 0)
	{
		scale = Vector3(1, 1, 1);
		rotation = Quaternion::Identity;
		translation = Vector3();
		return;
	}

	GetFrameIndex(time, KeyFrame);
	int nextFrame = KeyFrame + 1;
	if (nextFrame >= (int)pNode->Keys.size())
	{
		nextFrame = 0;
	}

	const AnimationKey& key = pNode->Keys[KeyFrame].Key;
	const AnimationKey& nextKey = pNode->Keys[nextFrame].Key;

	float t = time > 0.0f ? (time - pNode->Keys[KeyFrame].Time) / (pNode->Keys[nextFrame].Time - pNode->Keys[KeyFrame].Time) : 1.0f;

	translation = Vector3::Lerp(key.Position, nextKey.Position, t);
	scale = Vector3::Lerp(key.Scale, nextKey.Scale, t);
	rotation = Quaternion::Slerp(key.Rotation, nextKey.Rotation, t);
}

AnimationState::AnimationState(Animation* pAnimation, AnimatedModel* pModel)
	: m_pAnimation(pAnimation)
{
	m_pSkeleton = &pModel->GetSkeleton();
	for (const AnimationNode& node : pAnimation->GetNodes())
	{
		AnimationKeyState state;
		state.KeyFrame = 0;
		state.pBone = m_pSkeleton->GetBone(node.BoneIndex);
		state.pNode = &node;
		checkf(state.pBone->Name == state.pNode->Name, "[AnimationState::AnimationState] The name of the node and the bone should match");
		m_KeyStates.push_back(state);
	}
}

void AnimationState::AddTime(float time)
{
	float duration = GetDuration();
	if (duration == 0.0f || time == 0.0f)
	{
		return;
	}

	if (m_Looped)
	{
		m_Time = fmodf(m_Time + time, duration / GetAnimation()->GetTicksPerSecond());
	}
	else if (m_Time > duration)
	{
		return;
	}
	else
	{
		m_Time += time;
	}
	m_IsDirty = true;
}

void AnimationState::SetTime(float time)
{
	m_Time = time;
	m_IsDirty = true;
}

void AnimationState::Apply()
{
	if (m_IsDirty)
	{
		const std::vector<Bone>& bones = m_pSkeleton->GetBones();
		Vector3 translation, scale;
		Quaternion rotation;
		for (size_t i = 0; i < bones.size(); ++i)
		{
			Matrix m;
			m_KeyStates[i].GetTransform(m_Time, scale, rotation, translation);
			bones[i].pNode->SetLocalScaleSilent(scale);
			bones[i].pNode->SetLocalRotationSilent(rotation);
			bones[i].pNode->SetLocalPositionSilent(translation);
		}
		m_IsDirty = false;
	}
}

float AnimationState::GetDuration() const
{
	return m_pAnimation ? m_pAnimation->GetDurationInTicks() : 0.0f;
}