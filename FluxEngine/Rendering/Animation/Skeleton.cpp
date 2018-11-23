#include "FluxEngine.h"
#include "Skeleton.h"

Skeleton::Skeleton(const Skeleton& other)
	: m_RootBoneIndex(other.m_RootBoneIndex), m_Bones(other.m_Bones)
{
	for (Bone& bone : m_Bones)
	{
		bone.pNode = nullptr;
	}
}

Skeleton& Skeleton::operator=(const Skeleton& other)
{
	m_RootBoneIndex = other.m_RootBoneIndex;
	m_Bones = other.m_Bones;
	for (Bone& bone : m_Bones)
	{
		bone.pNode = nullptr;
	}
	return *this;
}

const Bone* Skeleton::GetBone(const int index) const
{
	check(index < (int)m_Bones.size());
	return &m_Bones[index];
}

Bone* Skeleton::GetBone(const int index)
{
	check(index < (int)m_Bones.size());
	return &m_Bones[index];
}

Bone* Skeleton::GetBone(const std::string& name)
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

void Skeleton::AddBone(const Bone& bone)
{
	m_Bones.push_back(bone);
}
