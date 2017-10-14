#include "stdafx.h"
#include "MeshRenderer.h"
#include "MeshFilter.h"
#include "Materials/Material.h"
#include "Scenegraph/GameObject.h"
#include "Core/Components/Transform.h"
#include "Rendering/Camera/Camera.h"

using namespace std;

MeshRenderer::MeshRenderer()
{
}

MeshRenderer::MeshRenderer(MeshFilter* pMeshFilter):
	m_pMeshFilter(pMeshFilter)
{
}

MeshRenderer::MeshRenderer(const string& meshPath)
{
	m_pMeshFilter = ResourceManager::Load<MeshFilter>(meshPath);
}

MeshRenderer::~MeshRenderer()
{

}

void MeshRenderer::SetMaterial(Material* pMaterial)
{
	m_pMaterial = pMaterial;
}

void MeshRenderer::Initialize()
{
	if (m_pMeshFilter)
	{
		m_pMeshFilter->Initialize(m_pGameContext);
		if (m_pMaterial)
		{ 
			if(m_pMaterial->IsInitialized() == false)
			{
				FLUX_LOG(WARNING, "[MeshRenderer::Initialize()] > Material not yet initialized!");
				return;
			}
			m_pMeshFilter->CreateBuffers(m_pMaterial->GetInputLayoutDesc());
		}

		BoundingOrientedBox::CreateFromPoints(m_LocalBoundingBox, m_pMeshFilter->VertexCount(), (Vector3*)m_pMeshFilter->GetVertexData("POSITION").pData, sizeof(Vector3));
	}
}

void MeshRenderer::Update()
{
	
}