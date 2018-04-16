#include "FluxEngine.h"
#include "AnimationState.h"
#include "Animation.h"
#include "AnimatedModel.h"

void AnimationKeyState::GetFrameIndex(float time, int& index) const
{
	if (time < 0.0f)
		time = 0.0f;
	if (index >= (int)pNode->Keys.size())
		index = (int)pNode->Keys.size() - 1;

	while (index && time < pNode->Keys[index].first)
		--index;
	while (index < (int)pNode->Keys.size() - 1 && time >= pNode->Keys[index + 1].first)
		++index;
}

void AnimationKeyState::GetMatrix(const float time, Matrix& matrix)
{
	GetFrameIndex(time, KeyFrame);
	int nextFrame = KeyFrame + 1;
	if (nextFrame >= (int)pNode->Keys.size())
		nextFrame = 0;

	const AnimationKey& key = pNode->Keys[KeyFrame].second;
	const AnimationKey& nextKey = pNode->Keys[nextFrame].second;

	float t = time > 0.0f ? (time - pNode->Keys[KeyFrame].first) / (pNode->Keys[nextFrame].first - pNode->Keys[KeyFrame].first) : 1.0f;

	const Vector3 position = Vector3::Lerp(key.Position, nextKey.Position, t);
	const Vector3 scale = Vector3::Lerp(key.Scale, nextKey.Scale, t);
	const Quaternion rotation = Quaternion::Lerp(key.Rotation, nextKey.Rotation, t);
	matrix = Matrix::CreateScale(scale) * Matrix::CreateFromQuaternion(rotation) * Matrix::CreateTranslation(position);
}

AnimationState::AnimationState(Animation* pAnimation, AnimatedModel* pModel) :
	m_pAnimation(pAnimation), m_pModel(pModel)
{
	const Skeleton& skeleton = pModel->GetSkeleton();
	m_pRootBone = skeleton.GetParentBone();
	for (const AnimationNode& node : pAnimation->GetNodes())
	{
		AnimationKeyState state;
		state.KeyFrame = 0;
		state.pBone = skeleton.GetBone(node.BoneIndex);
		state.pNode = &node;
		m_KeyStates.push_back(state);
	}
}

void AnimationState::AddTime(const float time)
{
	float duration = GetDuration();
	if (duration == 0.0f || time == 0.0f)
		return;

	if (m_Looped)
		m_Time = fmodf(m_Time + time, duration / GetAnimation()->GetTicksPerSecond());
	else if (m_Time > duration)
		return;
	else
		m_Time += time;
	m_IsDirty = true;
}

void AnimationState::SetTime(const float time)
{
	m_Time = time;
	m_IsDirty = false;
}

void AnimationState::Apply(std::vector<Matrix>& skinMatrices)
{
	if (m_IsDirty)
	{
		CalculateAnimations(m_pRootBone, Matrix(), skinMatrices);
		m_IsDirty = false;
	}
}

float AnimationState::GetDuration() const
{
	return m_pAnimation ? m_pAnimation->GetDurationInTicks() : 0.0f;
}

void AnimationState::CalculateAnimations(Bone* pBone, Matrix parentMatrix, std::vector<Matrix>& skinMatrices)
{
	if (m_pRootBone == nullptr)
		return;
	float time = m_Time * m_pAnimation->GetTicksPerSecond();
	AnimationKeyState& state = m_KeyStates[pBone->Index];
	Matrix m;
	if (m_KeyStates.size() != 0)
	{
		state.GetMatrix(time, m);
	}
	skinMatrices[pBone->Index] = pBone->OffsetMatrix * m * parentMatrix;

	for (Bone* pChild : pBone->Children)
		CalculateAnimations(pChild, m * parentMatrix, skinMatrices);
}

