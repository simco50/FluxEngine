#pragma once
struct Bone;
struct AnimationNode;
class AnimatedModel;
class Animation;

struct AnimationKeyState
{
	const Bone* pBone = nullptr;
	int KeyFrame = 0;
	const AnimationNode* pNode = nullptr;

	void GetFrameIndex(float time, int& index) const;
	void GetMatrix(const float time, Matrix& matrix);
};

class AnimationState
{
public:
	AnimationState(Animation* pAnimation, AnimatedModel* pModel);

	Animation* pAnimation = nullptr;
	AnimatedModel* pModel = nullptr;
	Bone* pRootBone = nullptr;
	std::vector<AnimationKeyState> KeyStates;
	bool IsDirty = true;

	float Time = 0.0f;

	void AddTime(const float time);

	void Apply(std::vector<Matrix>& skinMatrices);

private:
	void CalculateAnimations(Bone* pBone, Matrix parentMatrix, std::vector<Matrix>& skinMatrices);
};