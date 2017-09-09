#include "stdafx.h"
#include "MeshRenderer.h"
#include "../Graphics/MeshFilter.h"
#include "../Materials/Material.h"
#include "../Scenegraph/GameObject.h"
#include "Transform.h"
#include "Camera.h"

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
				Console::Log("[MeshRenderer::Initialize()] > Material not yet initialized!", LogType::ERROR);
				return;
			}
			m_pMeshFilter->CreateBuffers(m_pMaterial->GetInputLayoutDesc());
		}

		BoundingOrientedBox::CreateFromPoints(m_LocalBoundingBox, m_pMeshFilter->VertexCount(), (Vector3*)m_pMeshFilter->GetVertexData("POSITION").pData, sizeof(Vector3));
	}
}

void MeshRenderer::Update()
{
	if(m_pGameContext->Scene->Camera->FrustumCulling() && m_FrustumCulling)
		m_LocalBoundingBox.Transform(m_BoundingBox, XMLoadFloat4x4(&m_pGameObject->GetTransform()->GetWorldMatrix()));
}

void MeshRenderer::Render()
{
	if (!m_pMaterial)
		Console::Log("MeshRenderer::Render() -> Material not set", LogType::WARNING);
	else if(!m_pMeshFilter)
		Console::Log("MeshRenderer::Render() -> No MeshFilter", LogType::WARNING);
	else if (!m_FrustumCulling || m_pGameContext->Scene->Camera->IsInFrustum(this))
	{
		m_pMaterial->Update(this);
		m_pGameContext->Engine->D3DeviceContext->IASetIndexBuffer(m_pMeshFilter->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);
		UINT strides = m_pMaterial->GetInputLayoutDesc()->VertexStride;
		UINT offset = 0;
		ID3D11Buffer* pVertexBuffer = m_pMeshFilter->GetVertexBuffer();
		m_pGameContext->Engine->D3DeviceContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &strides, &offset);
		m_pGameContext->Engine->D3DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_pGameContext->Engine->D3DeviceContext->IASetInputLayout(m_pMaterial->GetInputLayout());

		D3DX11_TECHNIQUE_DESC techDesc;
		m_pMaterial->GetTechnique()->GetDesc(&techDesc);

		for (size_t i = 0; i < techDesc.Passes; i++)
		{
			m_pMaterial->GetTechnique()->GetPassByIndex(i)->Apply(0, m_pGameContext->Engine->D3DeviceContext);
			m_pGameContext->Engine->D3DeviceContext->DrawIndexed(m_pMeshFilter->IndexCount(), 0, 0);
		}
	}
}