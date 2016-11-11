#include "stdafx.h"
#include "FlexDebugRenderer.h"
#include "../Graphics/MeshFilter.h"
#include "../Components/CameraComponent.h"
#include "FlexHelper.h"
#include "FlexSystem.h"

FlexDebugRenderer::FlexDebugRenderer(FlexSystem* pFlexSystem) : m_pFlexSystem(pFlexSystem)
{
}


FlexDebugRenderer::~FlexDebugRenderer()
{
}

void FlexDebugRenderer::Initialize()
{
	m_pMeshFilter = ResourceManager::Load<MeshFilter>(L"./Resources/Meshes/FlexDebugSphere.flux");
	m_pMeshFilter->Initialize(m_pGameContext);

	CreateInputLayout();

	//Index Buffer
	CreateIndexBuffer();

	//Vertex Buffer
	CreateVertexBuffer();

	//Instance Buffer
	CreateInstanceData();

	//Get shader variables
	LoadShaderVariables();

	m_Particles.resize(m_pFlexSystem->Positions.size());
}

void FlexDebugRenderer::Update()
{
	if (m_Enabled == false)
		return;

	//Request the active particles from the solver
	m_ParticleCount = m_pFlexSystem->Positions.size();

	if (m_ParticleCount > m_MaxParticles)
	{
		DebugLog::Log(L"FlexDebugRenderer -> Buffer too small! Recreating buffer with appropriate size.", LogType::WARNING);
		m_MaxParticles = m_ParticleCount;
		CreateInstanceData();
	}

	for (int i = 0; i < m_ParticleCount; i++)
	{
		m_Particles[i].Position.x = m_pFlexSystem->Positions[i].x;
		m_Particles[i].Position.y = m_pFlexSystem->Positions[i].y;
		m_Particles[i].Position.z = m_pFlexSystem->Positions[i].z;
		m_Particles[i].Phase = m_pFlexSystem->Phases[i] & eFlexPhaseGroupMask;
	}

	D3D11_MAPPED_SUBRESOURCE mappedResouce;
	m_pGameContext->Engine->D3DeviceContext->Map(m_pInstanceBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResouce);
	memcpy(mappedResouce.pData, m_Particles.data(), sizeof(DebugParticle) * m_Particles.size());
	m_pGameContext->Engine->D3DeviceContext->Unmap(m_pInstanceBuffer.Get(), 0);
}

void FlexDebugRenderer::Render()
{
	if (m_Enabled == false)
		return;

	if (m_ParticleCount == 0)
		return;

	UpdateShaderVariables();

	ID3D11Buffer* vbs[] = { m_pVertexBuffer.Get(), m_pInstanceBuffer.Get() };
	UINT stride[2] = { sizeof(VertexPosNorm), sizeof(DebugParticle) };
	UINT offset[2] = { 0,0 };
	m_pGameContext->Engine->D3DeviceContext->IASetVertexBuffers(0, 2, vbs, stride, offset);
	m_pGameContext->Engine->D3DeviceContext->IASetInputLayout(m_pInputLayout.Get());
	m_pGameContext->Engine->D3DeviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pGameContext->Engine->D3DeviceContext->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	D3DX11_TECHNIQUE_DESC techDesc;
	m_pTechnique->GetDesc(&techDesc);
	for(uint32_t p = 0; p < techDesc.Passes; p++)
	{
		m_pTechnique->GetPassByIndex(p)->Apply(0, m_pGameContext->Engine->D3DeviceContext);
		m_pGameContext->Engine->D3DeviceContext->DrawIndexedInstanced(m_pMeshFilter->IndexCount(), m_ParticleCount, 0, 0, 0);
	}
}

void FlexDebugRenderer::SetMesh(const wstring& path)
{
	m_pMeshFilter = ResourceManager::Load<MeshFilter>(path);
	m_pMeshFilter->Initialize(m_pGameContext);

	CreateIndexBuffer();
	CreateVertexBuffer();
}

void FlexDebugRenderer::LoadShaderVariables()
{
	BIND_AND_CHECK_NAME(m_pColorVar, gColor, AsVector);
	m_pColorVar->GetFloatVector((float*)&m_Color);
	BIND_AND_CHECK_NAME(m_pLightDirectionVar, gLightDirection, AsVector);
	BIND_AND_CHECK_NAME(m_pVPVar, gViewProj, AsMatrix);
	BIND_AND_CHECK_NAME(m_pViewInverseVar, gViewInv, AsMatrix);
	BIND_AND_CHECK_NAME(m_pScaleVar, gScale, AsScalar);
}

void FlexDebugRenderer::UpdateShaderVariables()
{
	m_pColorVar->SetFloatVector(reinterpret_cast<const float*>(&m_Color));
	m_pVPVar->SetMatrix(reinterpret_cast<const float*>(&m_pGameContext->Scene->CurrentCamera->GetViewProjection()));
	m_pScaleVar->SetFloat(m_pFlexSystem->Params.mRadius * 0.5f * m_Scale);
	m_pViewInverseVar->SetMatrix(reinterpret_cast<const float*>(&m_pGameContext->Scene->CurrentCamera->GetViewInverse()));
}

void FlexDebugRenderer::CreateInputLayout()
{
	m_pEffect = ResourceManager::Load<ID3DX11Effect>(L"./Resources/Shaders/FleX/FlexDebug.fx");
	m_pTechnique = m_pEffect->GetTechniqueByIndex(0);

	D3D11_INPUT_ELEMENT_DESC elementDesc[] =
	{
		//Input Slot 0 (Vertex Data)
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		//Input Slot 1 (Instance Data)
		{ "WORLDPOS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "PHASE", 0, DXGI_FORMAT_R32_SINT, 1, 12, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
	};
	UINT numElements = sizeof(elementDesc) / sizeof(elementDesc[0]);

	D3DX11_PASS_DESC passDesc;
	m_pTechnique->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(m_pGameContext->Engine->D3Device->CreateInputLayout(elementDesc, numElements, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, m_pInputLayout.GetAddressOf()))
}

void FlexDebugRenderer::CreateIndexBuffer()
{
	m_pIndexBuffer.Reset();
	D3D11_BUFFER_DESC bd = {};
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.ByteWidth = sizeof(DWORD) * m_pMeshFilter->IndexCount();
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	bd.StructureByteStride = sizeof(DWORD);
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	D3D11_SUBRESOURCE_DATA data;

	data.pSysMem = m_pMeshFilter->GetVertexData("INDEX").pData;
	HR(m_pGameContext->Engine->D3Device->CreateBuffer(&bd, &data, m_pIndexBuffer.GetAddressOf()))
}

void FlexDebugRenderer::CreateVertexBuffer()
{
	m_pVertexBuffer.Reset();

	vector<VertexPosNorm> vertices;
	for (int i = 0; i < m_pMeshFilter->VertexCount(); ++i)
	{
		Vector3 pos = *((Vector3*)m_pMeshFilter->GetVertexData("POSITION").pData + i);
		Vector3 normal = *((Vector3*)m_pMeshFilter->GetVertexData("NORMAL").pData + i);
		vertices.push_back(VertexPosNorm(pos, normal));
	}
	D3D11_BUFFER_DESC bd = {};
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.ByteWidth = sizeof(VertexPosNorm) * vertices.size();
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	bd.StructureByteStride = sizeof(VertexPosNorm);
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = vertices.data();
	HR(m_pGameContext->Engine->D3Device->CreateBuffer(&bd, &data, m_pVertexBuffer.GetAddressOf()))
}

void FlexDebugRenderer::CreateInstanceData()
{
	m_pInstanceBuffer.Reset();

	m_MaxParticles = m_pFlexSystem->Positions.size();
	if (m_MaxParticles == 0)
		m_MaxParticles = 1;

	D3D11_BUFFER_DESC bd = {};
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.MiscFlags = 0;
	bd.StructureByteStride = sizeof(DebugParticle);
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bd.ByteWidth = sizeof(DebugParticle) * m_MaxParticles;
	HR(m_pGameContext->Engine->D3Device->CreateBuffer(&bd, nullptr, m_pInstanceBuffer.GetAddressOf()))
}
