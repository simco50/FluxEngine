#include "stdafx.h"
#include "FlexSoftbody.h"
#include "../../Physics/Flex/FlexHelper.h"
#include "../../Graphics/Texture.h"
#include "../../Materials/Flex/FlexSoftbodyMaterial.h"
#include "../../Graphics/MeshFilter.h"

FlexSoftbody::FlexSoftbody(const string& filePath, SoftbodyDesc* m_pSoftbodyDesc, FlexSystem* pFlexSystem):
FlexBody(filePath, pFlexSystem), m_pSoftbodyDesc(m_pSoftbodyDesc)
{
}

FlexSoftbody::~FlexSoftbody()
{
	SafeDelete(m_pMeshInstance);
	SafeDelete(m_pMaterial);
}

void FlexSoftbody::Initialize()
{
	LoadAndCreateBody();

	m_pMaterial = new FlexSoftbodyMaterial(m_pFlexSystem, m_pMeshInstance);
	m_pMaterial->Initialize(m_pGameContext);
	CreateSkinningBuffer();

	ILDesc desc;
	desc.InputLayoutDesc.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA,0 });
	desc.InputLayoutDesc.push_back({ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA,0 });
	desc.InputLayoutDesc.push_back({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA,0 });
	desc.VertexStride = 32;
	m_pMeshInstance->pMeshFilter->CreateBuffers(&desc);
}

void FlexSoftbody::Update()
{

}

void FlexSoftbody::Render()
{
	m_pMaterial->Update(nullptr);
	
	m_pGameContext->Engine->D3DeviceContext->IASetInputLayout(m_pMaterial->GetInputLayout());
	m_pGameContext->Engine->D3DeviceContext->IASetIndexBuffer(m_pMeshInstance->pMeshFilter->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);
	m_pGameContext->Engine->D3DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	UINT strides[2] = { sizeof(VertexPosNormTex), sizeof(FlexSkinVertex) };
	UINT offset[2] = { 0,0 };
	ID3D11Buffer* pBuffers[] = { m_pMeshInstance->pMeshFilter->GetVertexBuffer(), m_pSkinVertexBuffer.Get() };
	m_pGameContext->Engine->D3DeviceContext->IASetVertexBuffers(0, 2, pBuffers, strides, offset);

	D3DX11_TECHNIQUE_DESC techDesc;
	m_pMaterial->GetTechnique()->GetDesc(&techDesc);
	for (size_t p = 0; p < techDesc.Passes; p++)
	{
		m_pMaterial->GetTechnique()->GetPassByIndex(p)->Apply(0, m_pGameContext->Engine->D3DeviceContext);
		m_pGameContext->Engine->D3DeviceContext->DrawIndexed(m_pMeshInstance->pMeshFilter->IndexCount(), 0, 0);
	}
}

void FlexSoftbody::SetTexture(const string& filePath)
{
	m_pMaterial->SetTexture(ResourceManager::Load<Texture>(filePath));
}

void FlexSoftbody::CreateSkinningBuffer()
{
	m_pSkinVertexBuffer.Reset();
	D3D11_BUFFER_DESC bd = {};
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.ByteWidth = sizeof(FlexSkinVertex) * m_pMeshInstance->pMeshFilter->VertexCount();
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	bd.StructureByteStride = 0;
	bd.Usage = D3D11_USAGE_DEFAULT;
	D3D11_SUBRESOURCE_DATA initData;
	vector<FlexSkinVertex> vertices(m_pMeshInstance->pMeshFilter->VertexCount());
	for (size_t i = 0; i < vertices.size(); i++)
	{
		FlexSkinVertex v;
		for (size_t j = 0; j < 4; j++)
		{
			v.indices[j] = m_pMeshInstance->SkinningIndices[i * 4 + j];
			v.weights[j] = m_pMeshInstance->SkinningWeights[i * 4 + j];
		}
		vertices[i] = v;
	}
	initData.pSysMem = vertices.data();
	HR(m_pGameContext->Engine->D3Device->CreateBuffer(&bd, &initData, m_pSkinVertexBuffer.GetAddressOf()))
}

void FlexSoftbody::LoadAndCreateBody()
{
	m_pMeshInstance = new FlexHelper::FlexMeshInstance();
	m_pMeshInstance->pMeshFilter = ResourceManager::Load<MeshFilter>(m_FilePath);
	m_pMeshInstance->pMeshFilter->Initialize(m_pGameContext);
	CreateSoftbody();
	if (m_pMeshInstance->RigidRestPoses.size() > MAX_CLUSTERS)
		Console::LogFormat(LogType::ERROR, "FlexSoftbody::LoadAndCreateBody() > Mesh '%s' has too many rigid clusters! Max is %i. Try lowering the particle radius", m_FilePath.c_str(), MAX_CLUSTERS);
}

void FlexSoftbody::CreateSoftbody()
{
	PerfTimer perfTimer("CreateSoftBody");

	if (m_pSoftbodyDesc->IsValid() == false)
		Console::Log("CreateSoftBody() Failed! SoftbodyDesc not fully initialized!", LogType::ERROR);

	const float radius = m_pSoftbodyDesc->Radius;
	int vertexCount = m_pMeshInstance->pMeshFilter->GetVertexData("POSITION").Count;
	FlexExtAsset* pAsset = flexExtCreateSoftFromMesh(
		(const float*)m_pMeshInstance->pMeshFilter->GetVertexData("POSITION").pData,
		vertexCount,
		(const int*)m_pMeshInstance->pMeshFilter->GetVertexData("INDEX").pData,
		m_pMeshInstance->pMeshFilter->GetVertexData("INDEX").Count,
		radius,
		m_pSoftbodyDesc->VolumeSampling,
		m_pSoftbodyDesc->SurfaceSampling,
		m_pSoftbodyDesc->ClusterSpacing*radius,
		m_pSoftbodyDesc->ClusterRadius*radius,
		m_pSoftbodyDesc->ClusterStiffness,
		m_pSoftbodyDesc->LinkRadius*radius,
		m_pSoftbodyDesc->LinkStiffness
	);

	const int maxWeights = 4;
	m_pMeshInstance->SkinningIndices.resize(vertexCount * maxWeights);
	m_pMeshInstance->SkinningWeights.resize(vertexCount * maxWeights);

	for (int i = 0; i < pAsset->mNumShapes; i++)
		m_pMeshInstance->RigidRestPoses.push_back(Vector3(&pAsset->mShapeCenters[i * 3]));
	//Create the skinning
	flexExtCreateSoftMeshSkinning(
		(const float*)m_pMeshInstance->pMeshFilter->GetVertexData("POSITION").pData,
		vertexCount,
		pAsset->mShapeCenters,
		pAsset->mNumShapes,
		m_pSoftbodyDesc->SkinningFalloff,
		m_pSoftbodyDesc->SkinningMaxDistance,
		m_pMeshInstance->SkinningWeights.data(),
		m_pMeshInstance->SkinningIndices.data()
	);
	AddBodyFlexData(pAsset, m_pSoftbodyDesc->Phase);

	flexExtDestroyAsset(pAsset);

	perfTimer.Stop();
}
