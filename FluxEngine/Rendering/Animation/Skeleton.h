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

	Skeleton(const Skeleton& other)
		: m_RootBoneIndex(other.m_RootBoneIndex), m_Bones(other.m_Bones)
	{
		for (Bone& bone : m_Bones)
		{
			bone.pNode = nullptr;
		}
	}

	Skeleton& operator=(const Skeleton& other)
	{
		m_RootBoneIndex = other.m_RootBoneIndex;
		m_Bones = other.m_Bones;
		for (Bone& bone : m_Bones)
		{
			bone.pNode = nullptr;
		}
		return *this;
	}

	Bone* GetBone(const std::string& name)
	{
		for (Bone& bone : m_Bones)
		{
			if (bone.Name == name)
			{
				return &bone;
			}
		}
		return nullptr;
	}
	Bone* GetBone(const int index)
	{
		check(index < m_Bones.size());
		return &m_Bones[index];
	}

	const Bone* GetBone(const int index) const
	{
		check(index < m_Bones.size());
		return &m_Bones[index];
	}

	void AddBone(const Bone& bone)
	{
		m_Bones.push_back(bone);
	}

	const std::vector<Bone>& GetBones() const { return m_Bones; }
	int GetRootBoneIndex() const { return m_RootBoneIndex; }
	void SetParentBoneIndex(int boneIndex) { m_RootBoneIndex = boneIndex; }

	size_t BoneCount() const { return m_Bones.size(); }

	static const int MAX_BONE_COUNT = 100;

private:
	int m_RootBoneIndex = -1;
	std::vector<Bone> m_Bones;
};