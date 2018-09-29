#pragma once
#include "Content\Resource.h"

struct AnimationKey
{
	Vector3 Position;
	Quaternion Rotation;
	Vector3 Scale;
};

struct AnimationNode
{
	int BoneIndex = 0;
	std::string Name;
	struct KeyPair
	{
		KeyPair(float time, const AnimationKey& key)
			: Time(time), Key(key)
		{}
		float Time;
		AnimationKey Key;
	};

	std::vector<KeyPair> Keys;
};

class Animation : public Resource
{
	FLUX_OBJECT(Animation, Resource)

public:
	Animation(Context* pContext, const std::string& name, int numNodes, float duration, float ticksPerSecond);
	virtual ~Animation();

	virtual bool Load(InputStream& inputStream) override;

	void SetNode(const AnimationNode& node);
	AnimationNode& GetNode(int boneIndex);
	size_t GetNodeCount() const { return m_AnimationNodes.size(); }
	const std::vector<AnimationNode>& GetNodes() { return m_AnimationNodes; }

	const std::string& GetName() const { return m_Name; }
	StringHash GetNameHash() const { return m_NameHash; }
	float GetDurationInTicks() const { return m_DurationInTicks; }
	float GetTicksPerSecond() const { return m_TickPerSecond; }

private:
	std::string m_Name;
	StringHash m_NameHash;
	float m_DurationInTicks = 0.f;
	float m_TickPerSecond = 0.f;
	std::vector<AnimationNode> m_AnimationNodes;
};
