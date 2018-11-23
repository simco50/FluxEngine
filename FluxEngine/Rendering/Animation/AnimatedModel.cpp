#include "FluxEngine.h"
#include "AnimatedModel.h"
#include "Rendering/Mesh.h"
#include "Scenegraph/SceneNode.h"
#include "Animation.h"
#include "Rendering/Core/GraphicsDefines.h"

AnimatedModel::AnimatedModel(Context* pContext)
	: Model(pContext)
{
}

AnimatedModel::~AnimatedModel()
{
}

void AnimatedModel::Update()
{
	if (m_AnimationStates.size() > 0)
	{
		m_AnimationStates[0].Apply();
		m_pNode->MarkDirty();
	}

	ApplySkinning();
}

void AnimatedModel::SetMesh(Mesh* pMesh)
{
	checkf(m_pNode, "[AnimatedModel::SetMesh] Cannot set mesh when component is not attached to a node");
	checkf(pMesh->GetSkeleton().BoneCount() < GraphicsConstants::MAX_BONES, "[AnimatedModel::SetMesh] The given skeleton has more bones than allowed");

	const int geometries = pMesh->GetGeometryCount();
	m_Batches.resize(geometries);
	m_SkinMatrices.resize(pMesh->GetSkeleton().BoneCount());
	m_SkinQuaternions.resize(pMesh->GetSkeleton().BoneCount());
	for (int i = 0; i < geometries; ++i)
	{
		m_Batches[i].pGeometry = pMesh->GetGeometry(i);
		m_Batches[i].pWorldMatrices = m_SkinMatrices.data();
		m_Batches[i].NumSkinMatrices = (int)m_SkinMatrices.size();
		m_Batches[i].pSkinDualQuaternions = m_SkinQuaternions.data();
	}
	m_BoundingBox = pMesh->GetBoundingBox();

	m_pMesh = pMesh;
	SetSkeleton(m_pMesh->GetSkeleton());
}

void AnimatedModel::SetSkeleton(const Skeleton& skeleton)
{
	m_Skeleton = skeleton;
	AddBoneChildNodes(m_pNode, m_Skeleton.GetRootBoneIndex());
}

const Skeleton& AnimatedModel::GetSkeleton() const
{
	return m_Skeleton;
}

AnimationState* AnimatedModel::AddAnimationState(Animation* pAnimation)
{
	m_AnimationStates.push_back(AnimationState(pAnimation, this));
	return &m_AnimationStates.back();
}

void AnimatedModel::RemoveAnimationState(AnimationState* pAnimation)
{
	m_AnimationStates.erase(std::remove_if(m_AnimationStates.begin(), m_AnimationStates.end(), [pAnimation](const AnimationState& a) {return &a == pAnimation; }), m_AnimationStates.end());
}

AnimationState* AnimatedModel::GetAnimationState(const StringHash hash)
{
	for (AnimationState& state : m_AnimationStates)
	{
		if (state.GetAnimation()->GetNameHash() == hash)
		{
			return &state;
		}
	}
	return nullptr;
}

void AnimatedModel::ApplySkinning()
{
	const std::vector<Bone>& bones = m_Skeleton.GetBones();
	for (size_t i = 0; i < bones.size(); ++i)
	{
		m_SkinMatrices[i] = bones[i].OffsetMatrix * bones[i].pNode->GetWorldMatrix();
	}

/*#define DUALQ_SKINNING*/
#ifdef DUALQ_SKINNING
	//NOTE: I know this it's super dumb to decompose all the matrices,
	//make DQs and then copy them to the GPU on top of having matrix skinning but this is simply for DQ demonstration
	for (size_t i = 0; i < m_SkinMatrices.size(); ++i)
	{
		Vector3 position, scale;
		Quaternion rotation;
		m_SkinMatrices[i].Decompose(scale, rotation, position);
		m_SkinQuaternions[i] = DualQuaternion(rotation, position);
	}
#endif
}

void AnimatedModel::AddBoneChildNodes(SceneNode* pParentNode, int boneIndex)
{
	Bone* pBone = m_Skeleton.GetBone(boneIndex);
	SceneNode* pNewNode = pParentNode->CreateChild(pBone->Name);
	pBone->pNode = pNewNode;
	pBone->pNode->SetPosition(pBone->StartPosition, Space::Self);
	pBone->pNode->SetScale(pBone->StartScale, Space::Self);
	pBone->pNode->SetRotation(pBone->StartRotation, Space::Self);

	for (size_t i = 0; i < pBone->Children.size(); ++i)
	{
		AddBoneChildNodes(pNewNode, (int)pBone->Children[i]);
	}
}
