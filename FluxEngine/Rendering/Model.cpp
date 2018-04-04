#include "FluxEngine.h"
#include "Model.h"
#include "Scenegraph\Scene.h"
#include "Mesh.h"
#include "Scenegraph/SceneNode.h"
#include "SceneGraph/Transform.h"

Model::Model(Context* pContext):
	Drawable(pContext)
{

}

Model::~Model()
{
}

void Model::OnSceneSet(Scene* pScene)
{
	Drawable::OnSceneSet(pScene);
}

void Model::OnNodeSet(SceneNode* pNode)
{	
	Drawable::OnNodeSet(pNode);

	for (Batch& batch : m_Batches)
	{
		batch.pModelMatrix = &m_pNode->GetTransform()->GetWorldMatrix();
	}
}

void Model::OnMarkedDirty(const Transform* pTransform)
{
	for (Batch& batch : m_Batches)
		batch.pModelMatrix = &pTransform->GetWorldMatrix();
}

DirectX::SimpleMath::Matrix* Model::GetBoneMatrices()
{
	float time = GameTimer::GameTime();
	if (time != m_AnimationTime)
	{
		if (m_pMesh)
			m_pMesh->GetBoneMatrices(0, GameTimer::GameTime(), m_SkinMatrices);
		m_AnimationTime = time;
	}
	return m_SkinMatrices.data();
}

const Skeleton& Model::GetSkeleton() const
{
	return m_pMesh->GetSkeleton();
}

void Model::SetMesh(Mesh* pMesh)
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

void Model::SetMaterial(Material* pMaterial)
{
	for (Batch& batch : m_Batches)
	{
		batch.pMaterial = pMaterial;
	}
}

void Model::SetMaterial(int index, Material* pMaterial)
{
	if (index >= (int)m_Batches.size())
	{
		FLUX_LOG(Error, "[Model::SetMaterial] > Index out of range! Is '%i' but model only has '%i' batches", index, m_Batches.size());
		return;
	}
	m_Batches[index].pMaterial = pMaterial;
}