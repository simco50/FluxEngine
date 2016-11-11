#include "stdafx.h"
#include "FlexFluidRenderer.h"
#include "../Graphics/MeshFilter.h"
#include "../Components/CameraComponent.h"
#include "FlexHelper.h"
#include "FlexSystem.h"
#include "../../Graphics/RenderTarget.h"

FlexFluidRenderer::FlexFluidRenderer(FlexSystem* pFlexSystem) : m_pFlexSystem(pFlexSystem)
{
}


FlexFluidRenderer::~FlexFluidRenderer()
{
	SafeDelete(m_pRenderTarget);
}

void FlexFluidRenderer::Initialize()
{
	m_pMeshFilter = ResourceManager::Load<MeshFilter>(L"./Resources/Meshes/FlexDebugSphere.flux");
	m_pMeshFilter->Initialize(m_pGameContext);

	m_pRenderTarget = new RenderTarget(m_pGameContext->Engine);
	RENDER_TARGET_DESC desc;
	desc.DepthBuffer = true;
	desc.DepthSRV = true;
	desc.ColorSRV = true;
	desc.ColorFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.DepthFormat = DXGI_FORMAT_D32_FLOAT;
	desc.Width = m_pGameContext->Engine->GameSettings.Width;
	desc.Height = m_pGameContext->Engine->GameSettings.Height;
	m_pRenderTarget->Create(&desc);

	CreateInputLayout();
	CreateQuadInputLayout();

	//Index Buffer
	CreateIndexBuffer();

	//Vertex Buffer
	CreateVertexBuffer();
	CreateQuadVertexBuffer();

	//Instance Buffer
	CreateInstanceData();

	//Get shader variables
	LoadShaderVariables();

	m_Particles.resize(m_pFlexSystem->Positions.size());
}

void FlexFluidRenderer::Update()
{
	//Request the active particles from the solver
	m_ParticleCount = m_pFlexSystem->Positions.size();

	if (m_ParticleCount > m_MaxParticles)
	{
		DebugLog::Log(L"FlexFluidRenderer -> Buffer too small! Recreating buffer with appropriate size.", LogType::WARNING);
		m_MaxParticles = m_ParticleCount;
		CreateInstanceData();
	}

	D3D11_MAPPED_SUBRESOURCE mappedResouce;
	m_pGameContext->Engine->D3DeviceContext->Map(m_pInstanceBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResouce);
	memcpy(mappedResouce.pData, reinterpret_cast<float*>(m_pFlexSystem->Positions.data()), sizeof(Vector4) * m_ParticleCount);
	m_pGameContext->Engine->D3DeviceContext->Unmap(m_pInstanceBuffer.Get(), 0);
}

void FlexFluidRenderer::Render()
{
	if (m_ParticleCount == 0)
		return;

	m_pRenderTarget->ClearColor();
	m_pRenderTarget->ClearDepth();

	m_pDepthMapVariable->SetResource(nullptr);
	ID3D11RenderTargetView* pRtv = m_pRenderTarget->GetRenderTargetView();
	m_pGameContext->Engine->D3DeviceContext->OMSetRenderTargets(1, &pRtv, m_pRenderTarget->GetDepthStencilView());
	RenderSpheres();

	m_pDepthMapVariable->SetResource(m_pRenderTarget->GetDepthSRV());
	pRtv = m_pGameContext->Engine->DefaultRenderTarget->GetRenderTargetView();
	m_pGameContext->Engine->D3DeviceContext->OMSetRenderTargets(1, &pRtv, m_pGameContext->Engine->DefaultRenderTarget->GetDepthStencilView());
	RenderQuad();
}

void FlexFluidRenderer::RenderSpheres()
{
	m_pVPVar->SetMatrix(reinterpret_cast<const float*>(&m_pGameContext->Scene->CurrentCamera->GetViewProjection()));
	m_pScaleVar->SetFloat(m_pFlexSystem->Params.mRadius * 0.5f * m_Scale);

	//Render spheres
	ID3D11Buffer* vbs[] = { m_pVertexBuffer.Get(), m_pInstanceBuffer.Get() };
	UINT stride[2] = { sizeof(VertexPosNorm), sizeof(DebugParticle) };
	UINT offset[2] = { 0,0 };
	m_pGameContext->Engine->D3DeviceContext->IASetVertexBuffers(0, 2, vbs, stride, offset);
	m_pGameContext->Engine->D3DeviceContext->IASetInputLayout(m_pInputLayout.Get());
	m_pGameContext->Engine->D3DeviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pGameContext->Engine->D3DeviceContext->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	D3DX11_TECHNIQUE_DESC techDesc;
	m_pTechnique->GetDesc(&techDesc);
	for (uint32_t p = 0; p < techDesc.Passes; p++)
	{
		m_pTechnique->GetPassByIndex(p)->Apply(0, m_pGameContext->Engine->D3DeviceContext);
		m_pGameContext->Engine->D3DeviceContext->DrawIndexedInstanced(m_pMeshFilter->IndexCount(), m_ParticleCount, 0, 0, 0);
	}
}

void FlexFluidRenderer::RenderQuad()
{
	m_pViewInverseVar->SetMatrix((float*)&m_pGameContext->Scene->CurrentCamera->GetViewInverse());
	XMMATRIX mat = XMLoadFloat4x4(&m_pGameContext->Scene->CurrentCamera->GetProjection());
	mat = XMMatrixInverse(nullptr, mat);
	m_pProjectionInverseVar->SetMatrix((float*)&mat);

	//Render quad
	m_pGameContext->Engine->D3DeviceContext->IASetInputLayout(m_pQuadInputLayout.Get());

	UINT strides = sizeof(Vector3);
	UINT offset = 0;
	m_pGameContext->Engine->D3DeviceContext->IASetVertexBuffers(0, 1, m_pQuadVertexBuffer.GetAddressOf(), &strides, &offset);

	m_pGameContext->Engine->D3DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	D3DX11_TECHNIQUE_DESC techDesc;
	m_pQuadTechnique->GetDesc(&techDesc);
	for (size_t i = 0; i < techDesc.Passes; i++)
	{
		m_pQuadTechnique->GetPassByIndex(0)->Apply(0, m_pGameContext->Engine->D3DeviceContext);
		m_pGameContext->Engine->D3DeviceContext->Draw(4, 0);
	}
}

void FlexFluidRenderer::SetMesh(const wstring& path)
{
	m_pMeshFilter = ResourceManager::Load<MeshFilter>(path);
	m_pMeshFilter->Initialize(m_pGameContext);

	CreateIndexBuffer();
	CreateVertexBuffer();
}

void FlexFluidRenderer::LoadShaderVariables()
{
	//Spheres
	BIND_AND_CHECK_NAME(m_pVPVar, gViewProj, AsMatrix);
	BIND_AND_CHECK_NAME(m_pScaleVar, gScale, AsScalar);

	//Quad
	m_pDepthMapVariable = m_pQuadEffect->GetVariableByName("gDepth")->AsShaderResource();
	if (m_pDepthMapVariable == nullptr)
		DebugLog::LogFormat(LogType::ERROR, L"Variable with name 'gDepth' not found.");

	m_pLightDirectionVar = m_pQuadEffect->GetVariableByName("gLightDirection")->AsVector();
	if (m_pLightDirectionVar == nullptr)
		DebugLog::LogFormat(LogType::ERROR, L"Variable with name 'gLightDirection' not found.");

	m_pViewInverseVar = m_pQuadEffect->GetVariableByName("gViewInv")->AsMatrix();
	if (m_pViewInverseVar == nullptr)
		DebugLog::LogFormat(LogType::ERROR, L"Variable with name 'gViewInv' not found.");

	m_pProjectionInverseVar = m_pQuadEffect->GetVariableByName("gProjInv")->AsMatrix();
	if (m_pProjectionInverseVar == nullptr)
		DebugLog::LogFormat(LogType::ERROR, L"Variable with name 'gProjInv' not found.");
}

void FlexFluidRenderer::CreateInputLayout()
{
	m_pEffect = ResourceManager::Load<ID3DX11Effect>(L"./Resources/Shaders/Flex/FlexFluidSpherePass.fx");
	m_pTechnique = m_pEffect->GetTechniqueByIndex(0);

	D3D11_INPUT_ELEMENT_DESC elementDesc[] =
	{
		//Input Slot 0 (Vertex Data)
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		//Input Slot 1 (Instance Data)
		{ "WORLDPOS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
	};
	UINT numElements = sizeof(elementDesc) / sizeof(elementDesc[0]);

	D3DX11_PASS_DESC passDesc;
	m_pTechnique->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(m_pGameContext->Engine->D3Device->CreateInputLayout(elementDesc, numElements, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, m_pInputLayout.GetAddressOf()))
}

void FlexFluidRenderer::CreateIndexBuffer()
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

void FlexFluidRenderer::CreateVertexBuffer()
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

void FlexFluidRenderer::CreateInstanceData()
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

void FlexFluidRenderer::CreateQuadVertexBuffer()
{
	m_pQuadVertexBuffer.Reset();

	vector<Vector3> vertices(4);
	vertices[0] = Vector3(-1, 1, 0);
	vertices[1] = Vector3(1, 1, 0);
	vertices[2] = Vector3(-1, -1, 0);
	vertices[3] = Vector3(1, -1, 0);

	D3D11_BUFFER_DESC bd = {};
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.ByteWidth = sizeof(Vector3) * vertices.size();
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	bd.Usage = D3D11_USAGE_DEFAULT;

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = vertices.data();

	HR(m_pGameContext->Engine->D3Device->CreateBuffer(&bd, &initData, m_pQuadVertexBuffer.GetAddressOf()));
}

void FlexFluidRenderer::CreateQuadInputLayout()
{
	m_pQuadEffect = ResourceManager::Load<ID3DX11Effect>(L"./Resources/Shaders/FleX/FlexFluidQuadPass.fx");
	m_pQuadTechnique = m_pQuadEffect->GetTechniqueByIndex(0);

	D3D11_INPUT_ELEMENT_DESC elementDesc[] =
	{
		//Input Slot 0 (Vertex Data)
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT numElements = sizeof(elementDesc) / sizeof(elementDesc[0]);

	D3DX11_PASS_DESC passDesc;
	m_pQuadTechnique->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(m_pGameContext->Engine->D3Device->CreateInputLayout(elementDesc, numElements, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, m_pQuadInputLayout.GetAddressOf()))
}