#pragma once
struct Bone;
struct AnimationNode;
class AnimatedModel;
class Animation;
class Skeleton;

struct AnimationKeyState
{
	const Bone* pBone = nullptr;
	int KeyFrame = 0;
	const AnimationNode* pNode = nullptr;

	void GetFrameIndex(float time, int& index) const;
	void GetTransform(float time, Vector3& scale, Quaternion& rotation, Vector3& translation);
};

class AnimationState
{
public:
	AnimationState(Animation* pAnimation, AnimatedModel* pModel);

	void AddTime(float time);
	void SetTime(float time);
	float GetTime() const { return m_Time; }

	void Apply();
	void SetLoop(bool looped) { m_Looped = looped; }

	Animation* GetAnimation() const { return m_pAnimation; }
	bool IsLooped() const { return m_Looped; }
	float GetDuration() const;

private:
	float m_Time = 0.0f;
	bool m_IsDirty = true;
	bool m_Looped = true;
	Animation* m_pAnimation = nullptr;
	const Skeleton* m_pSkeleton = nullptr;
	std::vector<AnimationKeyState> m_KeyStates;
};