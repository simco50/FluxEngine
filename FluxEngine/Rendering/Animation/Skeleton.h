#pragma once

class SceneNode;

struct Bone
{
	std::string Name;
	Matrix OffsetMatrix;
	std::vector<int> Children;
	SceneNode* pNode = nullptr;

	Vector3 StartPosition;
	Vector3 StartScale;
	Quaternion StartRotation;
};

class Skeleton
{
public:
	Skeleton() = default;
	~Skeleton() = default;

	Skeleton(const Skeleton& other);
	Skeleton& operator=(const Skeleton& other);

	Bone* GetBone(const std::string& name);
	Bone* GetBone(const int index);
	const Bone* GetBone(const int index) const;

	void AddBone(const Bone& bone);

	const std::vector<Bone>& GetBones() const { return m_Bones; }
	int GetRootBoneIndex() const { return m_RootBoneIndex; }
	void SetParentBoneIndex(int boneIndex) { m_RootBoneIndex = boneIndex; }

	size_t BoneCount() const { return m_Bones.size(); }

private:
	int m_RootBoneIndex = -1;
	std::vector<Bone> m_Bones;
};