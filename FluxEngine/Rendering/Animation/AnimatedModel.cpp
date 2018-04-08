#include "FluxEngine.h"
#include "AnimatedModel.h"
#include "Scenegraph\Scene.h"
#include "Rendering/Mesh.h"
#include "Scenegraph/SceneNode.h"
#include "SceneGraph/Transform.h"
#include "Animation.h"

AnimatedModel::AnimatedModel(Context* pContext):
	Model(pContext)
{
	m_SkinMatrices.resize(Skeleton::MAX_BONE_COUNT);
}

AnimatedModel::~AnimatedModel()
{
}

void AnimatedModel::OnNodeSet(SceneNode* pNode)
{	
	Drawable::OnNodeSet(pNode);

	for (Batch& batch : m_Batches)
	{
		batch.pSkinMatrices = m_SkinMatrices.data();
		batch.NumSkinMatrices = (int)m_SkinMatrices.size();
		batch.pModelMatrix = &m_pNode->GetTransform()->GetWorldMatrix();
	}
}

void AnimatedModel::Update()
{

}

const Skeleton& AnimatedModel::GetSkeleton() const
{
	return m_pMesh->GetSkeleton();
}

AnimationState* AnimatedModel::AddAnimationState(Animation* pAnimation)
{
	AnimationState state(pAnimation, this);
	m_AnimationStates.push_back(state);
	return &m_AnimationStates.back();
}

void AnimatedModel::RemoveAnimationState(AnimationState* pAnimation)
{
	m_AnimationStates.erase(std::remove_if(m_AnimationStates.begin(), m_AnimationStates.end(), [pAnimation](const AnimationState& a) {return &a == pAnimation; }), m_AnimationStates.end());
}
