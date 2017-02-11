#include "stdafx.h"
#include "FluidRenderer.h"
#include "../Components/CameraComponent.h"
#include "FlexSystem.h"
#include "../../Graphics/RenderTarget.h"
#include <minwinbase.h>

FluidRenderer::FluidRenderer(FlexSystem* pFlexSystem) : m_pFlexSystem(pFlexSystem)
{
}


FluidRenderer::~FluidRenderer()
{
	delete m_pParticleRenderTarget;
}

void FluidRenderer::Initialize()
{
	m_MaxParticles = m_pFlexSystem->Positions.size();

	CreateRenderTarget();

	CreateInputLayouts();

	//Vertex Buffer
	CreateVertexBuffers();

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
		CreateVertexBuffers();
	}

	vector<Vector3> particles(m_ParticleCount);
	for (int i = 0; i < m_ParticleCount; i++)
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

	//Set the variables
	m_pViewInverseVar->SetMatrix(reinterpret_cast<const float*>(&m_pGameContext->Scene->CurrentCamera->GetViewInverse()));
	m_pVPVar->SetMatrix(reinterpret_cast<const float*>(&m_pGameContext->Scene->CurrentCamera->GetViewProjection()));

	m_pEffect->GetVariableByName("gProj")->AsMatrix()->SetMatrix(reinterpret_cast<const float*>(&m_pGameContext->Scene->CurrentCamera->GetProjection()));
	m_pEffect->GetVariableByName("gView")->AsMatrix()->SetMatrix(reinterpret_cast<const float*>(&m_pGameContext->Scene->CurrentCamera->GetView()));


	m_pScaleVar->SetFloat(m_pFlexSystem->Params.mRadius * m_Scale);
	//Clear the rendertarget
	m_pParticleRenderTarget->ClearColor();
	m_pParticleRenderTarget->ClearDepth();

	//Set the rendertarget
	ID3D11RenderTargetView* pRtv = m_pParticleRenderTarget->GetRenderTargetView();
	m_pGameContext->Engine->D3DeviceContext->OMSetRenderTargets(1, &pRtv, m_pParticleRenderTarget->GetDepthStencilView());
	m_pDepthMap->SetResource(nullptr);

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

	//Set the quad variables
	m_pDepthMap->SetResource(m_pParticleRenderTarget->GetColorSRV());
	m_pFluidEffect->GetVariableByName("gViewProjInv")->AsMatrix()->SetMatrix(reinterpret_cast<const float*>(&m_pGameContext->Scene->CurrentCamera->GetViewProjectionInverse()));

	//Set the default rendertarget
	pRtv = m_pGameContext->Engine->DefaultRenderTarget->GetRenderTargetView();
	m_pGameContext->Engine->D3DeviceContext->OMSetRenderTargets(1, &pRtv, m_pGameContext->Engine->DefaultRenderTarget->GetDepthStencilView());
	stride = sizeof(Vector3);
	offset = 0;
	m_pGameContext->Engine->D3DeviceContext->IASetVertexBuffers(0, 1, m_pQuadVertexBuffer.GetAddressOf(), &stride, &offset);
	m_pGameContext->Engine->D3DeviceContext->IASetInputLayout(m_pQuadInputLayout.Get());
	m_pGameContext->Engine->D3DeviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	m_pFluidTechnique->GetDesc(&techDesc);
	for (uint32_t p = 0; p < techDesc.Passes; p++)
	{
		m_pFluidTechnique->GetPassByIndex(p)->Apply(0, m_pGameContext->Engine->D3DeviceContext);
		m_pGameContext->Engine->D3DeviceContext->Draw(4, 0);
	}
}

void FluidRenderer::CreateRenderTarget()
{
	RENDER_TARGET_DESC desc;
	desc.Width = m_pGameContext->Engine->GameSettings.Width;
	desc.Height = m_pGameContext->Engine->GameSettings.Height;
	desc.ColorBuffer = true;
	desc.ColorSRV = true;
	desc.DepthBuffer = true;
	desc.DepthSRV = true;
	m_pParticleRenderTarget = new RenderTarget(m_pGameContext->Engine);
	m_pParticleRenderTarget->Create(&desc);
}

void FluidRenderer::LoadShaderVariables()
{
	BIND_AND_CHECK_NAME(m_pVPVar, gViewProj, AsMatrix);
	BIND_AND_CHECK_NAME(m_pScaleVar, gScale, AsScalar);
	BIND_AND_CHECK_NAME(m_pViewInverseVar, gViewInv, AsMatrix);

	m_pDepthMap = m_pFluidEffect->GetVariableByName("gDepth")->AsShaderResource();
	if (m_pDepthMap == nullptr)
		DebugLog::Log(L"FluidRenderer::LoadShaderVariables() > Variable 'gDepth' not found!", LogType::ERROR);
}

void FluidRenderer::CreateInputLayouts()
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

	m_pFluidEffect = ResourceManager::Load<ID3DX11Effect>(L"./Resources/Shaders/Flex/FlexFluidRenderer.fx");
	m_pFluidTechnique = m_pFluidEffect->GetTechniqueByIndex(0);

	m_pFluidTechnique->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(m_pGameContext->Engine->D3Device->CreateInputLayout(elementDesc, numElements, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, m_pQuadInputLayout.GetAddressOf()))
}

void FluidRenderer::CreateVertexBuffers()
{
	//Particles
	m_pVertexBuffer.Reset();
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(D3D11_BUFFER_DESC));
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.ByteWidth = sizeof(Vector3) * m_MaxParticles;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bd.MiscFlags = 0;
	bd.StructureByteStride = sizeof(Vector3);
	bd.Usage = D3D11_USAGE_DYNAMIC;
	HR(m_pGameContext->Engine->D3Device->CreateBuffer(&bd, nullptr, m_pVertexBuffer.GetAddressOf()))

	//Quad
	m_pQuadVertexBuffer.Reset();
	ZeroMemory(&bd, sizeof(D3D11_BUFFER_DESC));
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.ByteWidth = sizeof(Vector3) * 4;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	bd.StructureByteStride = sizeof(Vector3);
	bd.Usage = D3D11_USAGE_DEFAULT;

	vector<Vector3> vertices(4);
	vertices[0] = Vector3(-1, 1, 0);
	vertices[1] = Vector3(1, 1, 0);
	vertices[2] = Vector3(-1, -1, 0);
	vertices[3] = Vector3(1, -1, 0);

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = vertices.data();
	HR(m_pGameContext->Engine->D3Device->CreateBuffer(&bd, &initData, m_pQuadVertexBuffer.GetAddressOf()))
}