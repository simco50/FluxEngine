#pragma once
#include "Skeleton.h"

struct AnimationKey
{
	Vector3 Position;
	Quaternion Rotation;
	Vector3 Scale;
};

struct AnimationNode
{
	int BoneIndex;
	std::string Name;
	using KeyPair = std::pair<float, AnimationKey>;
	std::vector<KeyPair> Keys;

	int FrameIndex = 0;

	void GetFrameIndex(float time, int& index) const 
	{
		if (time < 0.0f)
			time = 0.0f;
		if (index >= (int)Keys.size())
			index = (int)Keys.size() - 1;

		while (index && time < Keys[index].first)
			--index;
		while (index < (int)Keys.size() - 1 && time >= Keys[index + 1].first)
			++index;
	}

	Matrix GetTransform(float time)
	{
		GetFrameIndex(time, FrameIndex);
		int nextFrame = FrameIndex + 1;
		if (nextFrame >= (int)Keys.size())
			nextFrame = 0;

		const AnimationKey& key = Keys[FrameIndex].second;
		const AnimationKey& nextKey = Keys[nextFrame].second;

		float t = time > 0.0f ? (time - Keys[FrameIndex].first) / (Keys[nextFrame].first - Keys[FrameIndex].first) : 1.0f;

		const Vector3 position = Vector3::Lerp(key.Position, nextKey.Position, t);
		//const Vector3 scale = Vector3::Lerp(key.Scale, nextKey.Scale, t);
		const Quaternion rotation = Quaternion::Lerp(key.Rotation, nextKey.Rotation, t);
		return Matrix::CreateFromQuaternion(rotation) * Matrix::CreateTranslation(position);
	}
};

class Animation
{
public:
	Animation(const std::string& name, int numNodes, const float duration, const float ticksPerSecond) :
		m_Name(name), m_DurationInTicks(duration), m_TickPerSecond(ticksPerSecond)
	{
		m_AnimationNodes.resize(numNodes);
	}
	~Animation() {}

	void SetNode(const AnimationNode& node) { m_AnimationNodes[node.BoneIndex] = node; }

	const std::string& GetName() const { return m_Name; }
	float GetDurationInTicks() const { return m_DurationInTicks; }
	float GetTicksPerSecond() const { return m_TickPerSecond; }

	void GetBoneMatrices(const float time, Skeleton& skeleton, std::vector<Matrix>& matrices)
	{
		Bone* pRoot = skeleton.GetParentBone();
		CalculateAnimations(fmod(time * m_TickPerSecond, m_DurationInTicks), pRoot, matrices, Matrix::CreateTranslation(0, 0, 0));
	}

	void CalculateAnimations(const float time, Bone* pBone, std::vector<Matrix>& matrices, Matrix parentMatrix)
	{
		if (pBone == nullptr)
			return;
		AnimationNode& node = m_AnimationNodes[pBone->Index];
		Matrix m;
		if (node.Keys.size() != 0)
		{
			m = node.GetTransform(time);
		}
		matrices[pBone->Index] = pBone->OffsetMatrix * m * parentMatrix;

		for (Bone* pChild : pBone->Children)
			CalculateAnimations(time, pChild, matrices, m * parentMatrix);
	}

private:
	std::string m_Name;
	float m_DurationInTicks = 0.f;
	float m_TickPerSecond = 0.f;
	std::vector<AnimationNode> m_AnimationNodes;
};
