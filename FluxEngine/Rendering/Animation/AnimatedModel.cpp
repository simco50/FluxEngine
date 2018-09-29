#include "FluxEngine.h"
#include "AnimatedModel.h"
#include "Rendering/Mesh.h"
#include "Scenegraph/SceneNode.h"
#include "SceneGraph/Transform.h"
#include "Animation.h"

AnimatedModel::AnimatedModel(Context* pContext):
	Model(pContext)
{
}

AnimatedModel::~AnimatedModel()
{
}

void AnimatedModel::Update()
{
	m_AnimationStates[0].Apply(m_SkinMatrices);
}

void AnimatedModel::SetMesh(Mesh* pMesh)
{
	if (m_pNode == nullptr)
	{
		FLUX_LOG(Warning, "[AnimatedModel::SetMesh] Cannot set mesh when component is not attached to a node");
		return;
	}

	const int geometries = pMesh->GetGeometryCount();
	m_Batches.resize(geometries);
	m_SkinMatrices.resize(pMesh->GetSkeleton().BoneCount());
	for (int i = 0; i < geometries; ++i)
	{
		m_Batches[i].pGeometry = pMesh->GetGeometry(i);
		m_Batches[i].pModelMatrix = &m_pNode->GetTransform()->GetWorldMatrix();
		m_Batches[i].pSkinMatrices = m_SkinMatrices.data();
		m_Batches[i].NumSkinMatrices = (int)m_SkinMatrices.size();
	}
	m_BoundingBox = pMesh->GetBoundingBox();

	m_pMesh = pMesh;
}

const Skeleton& AnimatedModel::GetSkeleton() const
{
	return m_pMesh->GetSkeleton();
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
