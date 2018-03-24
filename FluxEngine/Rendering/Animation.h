#pragma once

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
	std::vector<std::pair<float, AnimationKey>> Keys;
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

	std::vector<Matrix> GetBoneMatrices(const float time) const
	{
		UNREFERENCED_PARAMETER(time);
		std::vector<Matrix> boneMatrices;
		for (const AnimationNode& node : m_AnimationNodes)
		{
			if (node.Keys.size() == 0)
			{
				boneMatrices.push_back(Matrix());
			}
			else
			{
				const AnimationKey& key = node.Keys[(int)(GameTimer::GameTime() * 10) % node.Keys.size()].second;
				Matrix matrix = Matrix::CreateScale(key.Scale) * Matrix::CreateFromQuaternion(key.Rotation) * Matrix::CreateTranslation(key.Position);
				boneMatrices.push_back(matrix);
			}
		}
		return boneMatrices;
	}

private:
	std::string m_Name;
	float m_Duration = 0.f;
	float m_TickPerSecond = 0.f;
	std::vector<AnimationNode> m_AnimationNodes;
};
