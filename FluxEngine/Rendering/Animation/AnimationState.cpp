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
	pAnimation(pAnimation), pModel(pModel)
{
	const Skeleton& skeleton = pModel->GetSkeleton();
	pRootBone = skeleton.GetParentBone();
	for (const AnimationNode& node : pAnimation->GetNodes())
	{
		AnimationKeyState state;
		state.KeyFrame = 0;
		state.pBone = skeleton.GetBone(node.BoneIndex);
		state.pNode = &node;
		KeyStates.push_back(state);
	}
}

void AnimationState::AddTime(const float time)
{
	Time += time;
	IsDirty = true;
}

void AnimationState::Apply(std::vector<Matrix>& skinMatrices)
{
	if (IsDirty)
	{
		CalculateAnimations(pRootBone, Matrix(), skinMatrices);
		IsDirty = false;
	}
}

void AnimationState::CalculateAnimations(Bone* pBone, Matrix parentMatrix, std::vector<Matrix>& skinMatrices)
{
	if (pRootBone == nullptr)
		return;
	float time = fmod(Time * pAnimation->GetTicksPerSecond(), pAnimation->GetDurationInTicks());
	AnimationKeyState& state = KeyStates[pBone->Index];
	Matrix m;
	if (KeyStates.size() != 0)
	{
		state.GetMatrix(time, m);
	}
	skinMatrices[pBone->Index] = pBone->OffsetMatrix * m * parentMatrix;

	for (Bone* pChild : pBone->Children)
		CalculateAnimations(pChild, m * parentMatrix, skinMatrices);
}

