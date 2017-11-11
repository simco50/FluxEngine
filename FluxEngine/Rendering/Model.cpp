#include "stdafx.h"
#include "Model.h"
#include "Scenegraph\Scene.h"
#include "Mesh.h"

Model::Model()
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
}

void Model::SetMesh(Mesh* pMesh)
{
	int geometries = pMesh->GetGeometryCount();
	m_Batches.resize(geometries);
	for (int i = 0; i < geometries; ++i)
		m_Batches[i].pGeometry = pMesh->GetGeometry(i);
	m_pMesh = pMesh;
}

void Model::SetMaterial(Material* pMaterial)
{
	for (Batch& batch : m_Batches)
		batch.pMaterial = pMaterial;
}

void Model::SetMaterial(int index, Material* pMaterial)
{
	if (index >= m_Batches.size())
	{
		FLUX_LOG(ERROR, "[Model::SetMaterial] > Index out of range! Is '%i' but model only has '%i' batches", index, m_Batches.size());
		return;
	}
	m_Batches[index].pMaterial = pMaterial;
}
