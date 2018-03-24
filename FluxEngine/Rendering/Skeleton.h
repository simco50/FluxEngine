#pragma once

struct Bone
{
	int Index;
	std::string Name;
	Matrix OffsetMatrix;
};

class Skeleton
{
public:
	Skeleton() {}
	~Skeleton() {}

	Bone* GetBone(const std::string& name)
	{
		for (Bone& bone : m_Bones)
		{
			if (bone.Name == name)
				return &bone;
		}
		return nullptr;
	}
	Bone* GetBone(const int index)
	{
		for (Bone& bone : m_Bones)
		{
			if (bone.Index == index)
				return &bone;
		}
		return nullptr;
	}

	void AddBone(const Bone& bone)
	{
		m_Bones.push_back(bone);
	}

	size_t BoneCount() const { return m_Bones.size(); }

private:
	std::vector<Bone> m_Bones;
};