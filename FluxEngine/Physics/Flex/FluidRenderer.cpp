#include "stdafx.h"
#include "FluidRenderer.h"
#include "../Components/CameraComponent.h"
#include "FlexHelper.h"
#include "FlexSystem.h"

FluidRenderer::FluidRenderer(FlexSystem* pFlexSystem) : m_pFlexSystem(pFlexSystem)
{
}


FluidRenderer::~FluidRenderer()
{
}

void FluidRenderer::Initialize()
{
	m_MaxParticles = m_pFlexSystem->Positions.size();

	CreateInputLayout();

	//Vertex Buffer
	CreateVertexBuffer();

	//Get shader variables
	LoadShaderVariables();
}

void FluidRenderer::Update()
{
	//Request the active particles from the solver
	m_ParticleCount = m_pFlexSystem->Positions.size();
	if(m_ParticleCount > m_MaxParticles)
	{
		DebugLog::Log(L"FluidRenderer::Update() > Vertexbuffer too small! Recreating...");
		CreateVertexBuffer();
	}

	vector<Vector3> particles(m_ParticleCount);
	for (size_t i = 0; i < m_ParticleCount; i++)
		particles[i] = Vector3(m_pFlexSystem->Positions[i].x, m_pFlexSystem->Positions[i].y, m_pFlexSystem->Positions[i].z);

	D3D11_MAPPED_SUBRESOURCE resource;
	m_pGameContext->Engine->D3DeviceContext->Map(m_pVertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
	memcpy(resource.pData, particles.data(), particles.size() * sizeof(Vector3));
	m_pGameContext->Engine->D3DeviceContext->Unmap(m_pVertexBuffer.Get(), 0);

}

void FluidRenderer::Render()
{
	if (m_ParticleCount == 0)
		return;

	m_pViewInverseVar->SetMatrix(reinterpret_cast<const float*>(&m_pGameContext->Scene->CurrentCamera->GetViewInverse()));
	m_pVPVar->SetMatrix(reinterpret_cast<const float*>(&m_pGameContext->Scene->CurrentCamera->GetViewProjection()));
	m_pScaleVar->SetFloat(m_pFlexSystem->Params.mRadius * m_Scale);

	m_pEffect->GetVariableByName("gProj")->AsMatrix()->SetMatrix(reinterpret_cast<const float*>(&m_pGameContext->Scene->CurrentCamera->GetProjection()));

	//Render spheres
	UINT stride = sizeof(Vector3);
	UINT offset = 0;
	m_pGameContext->Engine->D3DeviceContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &stride, &offset);
	m_pGameContext->Engine->D3DeviceContext->IASetInputLayout(m_pInputLayout.Get());
	m_pGameContext->Engine->D3DeviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

	D3DX11_TECHNIQUE_DESC techDesc;
	m_pTechnique->GetDesc(&techDesc);
	for (uint32_t p = 0; p < techDesc.Passes; p++)
	{
		m_pTechnique->GetPassByIndex(p)->Apply(0, m_pGameContext->Engine->D3DeviceContext);
		m_pGameContext->Engine->D3DeviceContext->Draw(m_ParticleCount, 0);
	}
}

void FluidRenderer::LoadShaderVariables()
{
	BIND_AND_CHECK_NAME(m_pVPVar, gViewProj, AsMatrix);
	BIND_AND_CHECK_NAME(m_pScaleVar, gScale, AsScalar);
	BIND_AND_CHECK_NAME(m_pViewInverseVar, gViewInv, AsMatrix);
}

void FluidRenderer::CreateInputLayout()
{
	m_pEffect = ResourceManager::Load<ID3DX11Effect>(L"./Resources/Shaders/Flex/FlexQuadRenderer.fx");
	m_pTechnique = m_pEffect->GetTechniqueByIndex(0);

	D3D11_INPUT_ELEMENT_DESC elementDesc[] =
	{
		//Input Slot 0 (Vertex Data)
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT numElements = sizeof(elementDesc) / sizeof(elementDesc[0]);

	D3DX11_PASS_DESC passDesc;
	m_pTechnique->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(m_pGameContext->Engine->D3Device->CreateInputLayout(elementDesc, numElements, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, m_pInputLayout.GetAddressOf()))
}

void FluidRenderer::CreateVertexBuffer()
{
	m_pVertexBuffer.Reset();
	D3D11_BUFFER_DESC bd = {};
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.ByteWidth = sizeof(Vector3) * m_MaxParticles;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bd.MiscFlags = 0;
	bd.StructureByteStride = sizeof(Vector3);
	bd.Usage = D3D11_USAGE_DYNAMIC;
	HR(m_pGameContext->Engine->D3Device->CreateBuffer(&bd, nullptr, m_pVertexBuffer.GetAddressOf()))
}