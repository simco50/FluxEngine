#include "stdafx.h"
#include "FlexRigidbody.h"
#include "FlexHelper.h"
#include "../../Graphics/Texture.h"
#include "../../Materials/Flex/FlexRigidbodyMaterial.h"
#include "../../Graphics/MeshFilter.h"

FlexRigidbody::FlexRigidbody(const string &filePath, RigidbodyDesc* m_pRigidbodyDesc, FlexSystem* pFlexSystem) :
FlexBody(filePath, pFlexSystem), m_pRigidbodyDesc(m_pRigidbodyDesc)
{
}


FlexRigidbody::~FlexRigidbody()
{
	SafeDelete(m_pMeshInstance);
	SafeDelete(m_pMaterial);
}

void FlexRigidbody::Initialize()
{
	LoadAndCreateBody();
	m_pMaterial = new FlexRigidbodyMaterial(m_pFlexSystem, m_pMeshInstance);
	m_pMaterial->Initialize(m_pGameContext);
	m_pMeshInstance->pMeshFilter->CreateBuffers(m_pMaterial);
}

void FlexRigidbody::Update()
{

}

void FlexRigidbody::Render()
{
	m_pMaterial->Update(nullptr);

	m_pGameContext->Engine->D3DeviceContext->IASetInputLayout(m_pMaterial->GetInputLayout());
	m_pGameContext->Engine->D3DeviceContext->IASetIndexBuffer(m_pMeshInstance->pMeshFilter->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);
	m_pGameContext->Engine->D3DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT strides = m_pMaterial->GetInputLayoutDesc()->VertexStride;
	UINT offset = 0;
	ID3D11Buffer* pBuffer = m_pMeshInstance->pMeshFilter->GetVertexBuffer();
	m_pGameContext->Engine->D3DeviceContext->IASetVertexBuffers(0, 1, &pBuffer, &strides, &offset);

	D3DX11_TECHNIQUE_DESC techDesc;
	m_pMaterial->GetTechnique()->GetDesc(&techDesc);
	for (size_t p = 0; p < techDesc.Passes; p++)
	{
		m_pMaterial->GetTechnique()->GetPassByIndex(p)->Apply(0, m_pGameContext->Engine->D3DeviceContext);
		m_pGameContext->Engine->D3DeviceContext->DrawIndexed(m_pMeshInstance->pMeshFilter->IndexCount(), 0, 0);
	}
}

void FlexRigidbody::LoadAndCreateBody()
{
	m_pMeshInstance = new FlexHelper::FlexMeshInstance();
	m_pMeshInstance->pMeshFilter = ResourceManager::Load<MeshFilter>(m_FilePath);
	m_pMeshInstance->pMeshFilter->Initialize(m_pGameContext);
	CreateRigidBody();
}

void FlexRigidbody::SetTexture(const string& filePath)
{
	m_pMaterial->SetTexture(ResourceManager::Load<Texture>(filePath));
}

void FlexRigidbody::CreateRigidBody()
{
	PerfTimer perfTimer("CreateRigidBody");

	if (m_pRigidbodyDesc->IsValid() == false)
		Console::Log("CreateRigidBody() Failed! RigidbodyDesc not fully initialized!", LogType::ERROR);

	FlexExtAsset* pAsset = flexExtCreateRigidFromMesh(
		(const float*)m_pMeshInstance->pMeshFilter->GetVertexData("POSITION").pData,
		m_pMeshInstance->pMeshFilter->VertexCount(),
		(const int*)m_pMeshInstance->pMeshFilter->GetVertexData("INDEX").pData,
		m_pMeshInstance->pMeshFilter->IndexCount(),
		m_pRigidbodyDesc->Radius,
		m_pRigidbodyDesc->Expand
	);

	for (int i = 0; i < pAsset->mNumShapes; i++)
		m_pMeshInstance->RigidRestPoses.push_back(Vector3(&pAsset->mShapeCenters[i * 3]));

	AddBodyFlexData(pAsset, m_pRigidbodyDesc->Phase);

	flexExtDestroyAsset(pAsset);

	perfTimer.Stop();
}