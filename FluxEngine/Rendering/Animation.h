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

	Matrix GetTransform(float time) const
	{
		if (time < 0)
			time = 0;
		if (Keys.size() == 0)
			return Matrix::CreateTranslation(0, 0, 0);
		if(Keys.size() == 1)
			return Matrix::CreateScale(Keys[0].second.Scale) * Matrix::CreateFromQuaternion(Keys[0].second.Rotation) * Matrix::CreateTranslation(Keys[0].second.Position);
		for (int i = 0; i < Keys.size() ; i++)
		{
			if(time == Keys[i].first)
				return Matrix::CreateScale(Keys[i].second.Scale) * Matrix::CreateFromQuaternion(Keys[i].second.Rotation) * Matrix::CreateTranslation(Keys[i].second.Position);
			if (time < Keys[i].first)
			{
				Vector3 position = Vector3::Lerp(Keys[i - 1].second.Position, Keys[i].second.Position, Keys[i].first - time);
				Vector3 scale = Vector3::Lerp(Keys[i - 1].second.Scale, Keys[i].second.Scale, Keys[i].first - time);
				Quaternion rotation = Quaternion::Lerp(Keys[i - 1].second.Rotation, Keys[i].second.Rotation, Keys[i].first - time);
				return Matrix::CreateScale(scale) * Matrix::CreateFromQuaternion(rotation) * Matrix::CreateTranslation(position);
			}
		}
		return Matrix::CreateScale(Keys.back().second.Scale) * Matrix::CreateFromQuaternion(Keys.back().second.Rotation) * Matrix::CreateTranslation(Keys.back().second.Position);
	}
};

class Animation
{
public:
	Animation(const std::string& name, int numNodes, const float duration, const float ticksPerSecond) :
		m_Name(name), m_Duration(duration), m_TickPerSecond(ticksPerSecond)
	{
		m_AnimationNodes.resize(numNodes);
	}
	~Animation() {}

	void SetNode(const int boneIndex, const AnimationNode& node) { m_AnimationNodes[boneIndex] = node; }

	const std::string& GetName() const { return m_Name; }
	float GetDuration() const { return m_Duration; }
	float GetTicksPerSecond() const { return m_TickPerSecond; }

	std::vector<Matrix> GetBoneMatrices(const float time, Skeleton& skeleton)
	{
		std::vector<Matrix> boneMatrices;

		Bone* pRoot = skeleton.GetParentBone();

		boneMatrices.resize(/*m_AnimationNodes.size()*/ 100);
		CalculateAnimations(fmod(time, m_Duration), pRoot, boneMatrices, Matrix::CreateTranslation(0, 0, 0));

		return boneMatrices;
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
		matrices[pBone->Index] = pBone->OffsetMatrix *m *  parentMatrix;

		for (Bone* pChild : pBone->Children)
			CalculateAnimations(time, pChild, matrices, m * parentMatrix);
	}

private:
	std::string m_Name;
	float m_Duration = 0.f;
	float m_TickPerSecond = 0.f;
	std::vector<AnimationNode> m_AnimationNodes;
};
