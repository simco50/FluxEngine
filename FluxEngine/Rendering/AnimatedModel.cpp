#include "FluxEngine.h"
#include "AnimatedModel.h"
#include "Scenegraph\Scene.h"
#include "Mesh.h"
#include "Scenegraph/SceneNode.h"
#include "SceneGraph/Transform.h"

AnimatedModel::AnimatedModel(Context* pContext):
	Model(pContext)
{

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
		batch.NumSkinMatrices = m_SkinMatrices.size();
		batch.pModelMatrix = &m_pNode->GetTransform()->GetWorldMatrix();
	}
}

void AnimatedModel::Update()
{
	ApplyAnimation();
}

void AnimatedModel::ApplyAnimation()
{
	float time = GameTimer::GameTime();
	if (time != m_AnimationTime)
	{
		if (m_pMesh)
			m_pMesh->GetBoneMatrices(0, GameTimer::GameTime(), m_SkinMatrices);
		m_AnimationTime = time;
	}
}

const Skeleton& AnimatedModel::GetSkeleton() const
{
	return m_pMesh->GetSkeleton();
}

void AnimatedModel::SetMesh(Mesh* pMesh)
{
	if (pMesh)
	{
		int geometries = pMesh->GetGeometryCount();
		m_Batches.resize(geometries);
		for (int i = 0; i < geometries; ++i)
			m_Batches[i].pGeometry = pMesh->GetGeometry(i);
		m_BoundingBox = pMesh->GetBoundingBox();
	}
	else
	{
		m_BoundingBox = BoundingBox();
	}
	m_pMesh = pMesh;
	m_SkinMatrices.resize(Skeleton::MAX_BONE_COUNT);
}