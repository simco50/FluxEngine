//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

#include "ParticleEmitterComponent.h"
#include "../Content/ResourceManager.h"
#include "../Graphics/Particle.h"
#include "../Components/TransformComponent.h"
#include "../Debugging/DebugLog.h"
#include "../Graphics/Texture.h"
#include "CameraComponent.h"

ParticleEmitterComponent::ParticleEmitterComponent(const wstring& assetFile, int particleCount) :
	m_ParticleCount(particleCount),
	m_AssetFile(assetFile)
{
	m_Particles.resize(m_ParticleCount);
	for (int i = 0; i < m_ParticleCount; i++)
		m_Particles[i] = new Particle(&m_Settings);
}


ParticleEmitterComponent::~ParticleEmitterComponent(void)
{
	for (size_t i = 0; i < m_Particles.size(); i++)
		delete m_Particles[i];
	m_Particles.clear();

	SafeRelease(m_pInputLayout);
	SafeRelease(m_pVertexBuffer);
}

void ParticleEmitterComponent::Initialize()
{
	LoadEffect();
	CreateVertexBuffer();
	m_pParticleTexture = ResourceManager::Load<Texture>(m_AssetFile);
}

void ParticleEmitterComponent::LoadEffect()
{
	m_pEffect = ResourceManager::Load<ID3DX11Effect>(L"./Resources/Shaders/ParticleRenderer.fx");
	m_pDefaultTechnique = m_pEffect->GetTechniqueByIndex(0);

	BIND_AND_CHECK_NAME(m_pWvpVariable, gWorldViewProj, AsMatrix);
	BIND_AND_CHECK_NAME(m_pViewInverseVariable, gViewInverse, AsMatrix);	
	BIND_AND_CHECK_NAME(m_pTextureVariable, gParticleTexture, AsShaderResource);

	D3D11_INPUT_ELEMENT_DESC desc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12,D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32_FLOAT, 0, 28,D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 1, DXGI_FORMAT_R32_FLOAT, 0, 32,D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	D3DX11_PASS_DESC passDesc;
	m_pDefaultTechnique->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(m_pGameContext->Engine->D3Device->CreateInputLayout(desc, 4, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &m_pInputLayout));
}

void ParticleEmitterComponent::CreateVertexBuffer()
{
	if (m_pVertexBuffer)
		SafeRelease(m_pVertexBuffer);

	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = m_ParticleCount * sizeof(ParticleVertex);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bd.MiscFlags = 0;
	HRESULT hr = m_pGameContext->Engine->D3Device->CreateBuffer(&bd, nullptr, &m_pVertexBuffer);
	DebugLog::LogHRESULT(L"ParticleEmitterComponent::CreateVertexBuffer...", hr);
}

void ParticleEmitterComponent::Update()
{
	float particleInterval = m_Settings.Lifetime / m_ParticleCount;

	m_LastParticleInit += GameTimer::DeltaTime();
	m_ActiveParticles = 0;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	m_pGameContext->Engine->D3DeviceContext->Map(m_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	ParticleVertex* pBuffer = static_cast<ParticleVertex*>(mappedResource.pData);
	for (Particle* p : m_Particles)
	{
		p->Update();
		if (p->IsActive())
		{
			pBuffer[m_ActiveParticles] = p->GetVertexInfo();
			++m_ActiveParticles;
		}
		else if(m_LastParticleInit >= particleInterval)
		{
			p->Init(GetTransform()->GetWorldPosition());
			pBuffer[m_ActiveParticles] = p->GetVertexInfo();
			++m_ActiveParticles;
			m_LastParticleInit = 0.0f;
		}
	}
	m_pGameContext->Engine->D3DeviceContext->Unmap(m_pVertexBuffer, 0);
}

void ParticleEmitterComponent::Render()
{
	XMFLOAT4X4 viewProjection = m_pGameContext->Scene->CurrentCamera->GetViewProjection();
	XMFLOAT4X4 viewInverse = m_pGameContext->Scene->CurrentCamera->GetViewInverse();
	m_pWvpVariable->SetMatrix(reinterpret_cast<float*>(&viewProjection));
	m_pViewInverseVariable->SetMatrix(reinterpret_cast<float*>(&viewInverse));
	m_pTextureVariable->SetResource(m_pParticleTexture->GetResourceView());

	m_pGameContext->Engine->D3DeviceContext->IASetInputLayout(m_pInputLayout);
	m_pGameContext->Engine->D3DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	UINT strides = sizeof(ParticleVertex);
	UINT offset = 0;
	m_pGameContext->Engine->D3DeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &strides, &offset);

	D3DX11_TECHNIQUE_DESC techDesc;
	m_pDefaultTechnique->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		m_pDefaultTechnique->GetPassByIndex(p)->Apply(0, m_pGameContext->Engine->D3DeviceContext);
		m_pGameContext->Engine->D3DeviceContext->Draw(m_ActiveParticles, 0);
	}
}
