#pragma once

struct Bone
{
	int Index = 0;
	std::string Name;
	Matrix OffsetMatrix;
	std::vector<Bone*> Children;
};

class Skeleton
{
public:
	Skeleton() = default;
	~Skeleton() = default;

	DELETE_COPY(Skeleton);

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
		for (Bone& bone : m_Bones)
		{
			if (bone.Index == index)
			{
				return &bone;
			}
		}
		return nullptr;
	}

	const Bone* GetBone(const int index) const
	{
		for (const Bone& bone : m_Bones)
		{
			if (bone.Index == index)
			{
				return &bone;
			}
		}
		return nullptr;
	}

	void AddBone(const Bone& bone)
	{
		m_Bones.push_back(bone);
	}

	const std::vector<Bone>& GetBones() const { return m_Bones; }
	const Bone* GetParentBone() const { return GetBone(m_RootBoneIndex); }
	void SetParentBoneIndex(int boneIndex) { m_RootBoneIndex = boneIndex; }

	size_t BoneCount() const { return m_Bones.size(); }

	static const int MAX_BONE_COUNT = 100;

private:
	int m_RootBoneIndex = -1;
	std::vector<Bone> m_Bones;
};