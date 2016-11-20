//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

#include "ParticleEmitterComponent.h"
#include "../Content/ResourceManager.h"
#include "../Graphics/Particle.h"
#include "../Components/TransformComponent.h"
#include "../Debugging/DebugLog.h"
#include "../Graphics/Texture.h"
#include "CameraComponent.h"
#include <minwinbase.h>

ParticleEmitterComponent::ParticleEmitterComponent(const wstring& assetFile, int emission, int maxParticles, bool playOnAwake) :
	m_MaxParticles(maxParticles),
	m_Emission(emission),
	m_AssetFile(assetFile),
	m_PlayOnAwake(playOnAwake)
{
	m_Particles.resize(maxParticles);
	for (int i = 0; i < maxParticles; i++)
		m_Particles[i] = new Particle(&m_Settings);
}


ParticleEmitterComponent::~ParticleEmitterComponent(void)
{
	for (size_t i = 0; i < m_Particles.size(); i++)
		delete m_Particles[i];
	m_Particles.clear();
}

void ParticleEmitterComponent::SetMaxParticles(const int maxParticles)
{
	m_MaxParticles = maxParticles;
	CreateVertexBuffer();
}

void ParticleEmitterComponent::Initialize()
{
	LoadEffect();
	CreateVertexBuffer();
	CreateBlendState();
	m_pParticleTexture = ResourceManager::Load<Texture>(m_AssetFile);

	if (m_PlayOnAwake)
		Play();
}

void ParticleEmitterComponent::LoadEffect()
{
	m_pEffect = ResourceManager::Load<ID3DX11Effect>(L"./Resources/Shaders/ParticleRenderer.fx");
	m_pTechnique = m_pEffect->GetTechniqueByIndex(0);

	BIND_AND_CHECK_NAME(m_pViewProjectionVariable, gViewProj, AsMatrix);
	BIND_AND_CHECK_NAME(m_pWorldVariable, gWorld, AsMatrix);
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
	UINT numElements = sizeof(desc) / sizeof(desc[0]);

	m_pTechnique->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(m_pGameContext->Engine->D3Device->CreateInputLayout(desc, numElements, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, m_pInputLayout.GetAddressOf()));
}

void ParticleEmitterComponent::CreateVertexBuffer()
{
	m_pVertexBuffer.Reset();

	m_BufferSize = m_MaxParticles;

	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = m_BufferSize * sizeof(ParticleVertex);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bd.MiscFlags = 0;
	HRESULT hr = m_pGameContext->Engine->D3Device->CreateBuffer(&bd, nullptr, m_pVertexBuffer.GetAddressOf());
	DebugLog::LogHRESULT(L"ParticleEmitterComponent::CreateVertexBuffer...", hr);
}

void ParticleEmitterComponent::CreateBlendState()
{
	m_pBlendState.Reset();
	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));
	blendDesc.RenderTarget[0].BlendEnable = true;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = 0x0f;
	HR(m_pGameContext->Engine->D3Device->CreateBlendState(&blendDesc, m_pBlendState.GetAddressOf()))
}

/*bool ParticleEmitterComponent::SortByCameraDistance(Particle* p1, Particle* p2)
{
	float d1 = Vector3::DistanceSquared(p1->GetVertexInfo().Position, m_pGameContext->Scene->CurrentCamera->GetTransform()->GetWorldPosition());
	float d2 = Vector3::DistanceSquared(p2->GetVertexInfo().Position, m_pGameContext->Scene->CurrentCamera->GetTransform()->GetWorldPosition());
	return d1 > d2;
}*/

void ParticleEmitterComponent::Update()
{
	if (m_Playing == false)
		return;

	m_Timer += GameTimer::DeltaTime();
	bool update = true;
	if (m_Timer > m_Duration && m_Loop == false)
		update = false;

	if (m_ActiveParticles == 0 && update == false)
		return;

	float emissionTime = 1.0f / m_Emission;
	m_ParticleSpawnTimer += GameTimer::DeltaTime();

	m_ActiveParticles = 0;
	m_ParticleCount = 0;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	m_pGameContext->Engine->D3DeviceContext->Map(m_pVertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	ParticleVertex* pBuffer = static_cast<ParticleVertex*>(mappedResource.pData);

	if (m_SortingMode == ParticleSortingMode::ByDistance)
	{
		sort(m_Particles.begin(), m_Particles.end(), [this](Particle* p1, Particle* p2)
		{

			float d1 = Vector3::DistanceSquared(p1->GetVertexInfo().Position, m_pGameContext->Scene->Cameras[0]->GetTransform()->GetWorldPosition());
			float d2 = Vector3::DistanceSquared(p2->GetVertexInfo().Position, m_pGameContext->Scene->Cameras[0]->GetTransform()->GetWorldPosition());
			return d1 > d2;
		});
	}
	bool oldFirst = m_SortingMode == ParticleSortingMode::OldestFirst;
	
	for (size_t i = 0; i < m_Particles.size(); i++)
	{
		Particle* p = oldFirst ? m_Particles[m_Particles.size() - 1 - i] : m_Particles[i];
		if (p->IsActive())
		{
			p->Update();
			pBuffer[m_ParticleCount] = p->GetVertexInfo();
			++m_ParticleCount;
			++m_ActiveParticles;
		}
		else if(m_ParticleSpawnTimer >= emissionTime && update)
		{
			p->Init();
			pBuffer[m_ParticleCount] = p->GetVertexInfo();
			++m_ParticleCount;
			m_ParticleSpawnTimer = 0.0f;
		}
	}
	m_pGameContext->Engine->D3DeviceContext->Unmap(m_pVertexBuffer.Get(), 0);
}

void ParticleEmitterComponent::Render()
{
	if (m_Playing == false)
		return;

	if(m_MaxParticles > m_BufferSize)
	{
		m_BufferSize = m_MaxParticles;
		DebugLog::Log(L"ParticleEmitterComponent::Render() > VertexBuffer too small! Increasing size...", LogType::WARNING);
		CreateVertexBuffer();
	}

	XMFLOAT4X4 viewProjection = m_pGameContext->Scene->CurrentCamera->GetViewProjection();
	XMFLOAT4X4 viewInverse = m_pGameContext->Scene->CurrentCamera->GetViewInverse();

	m_pViewProjectionVariable->SetMatrix(reinterpret_cast<float*>(&viewProjection));
	m_pViewInverseVariable->SetMatrix(reinterpret_cast<float*>(&viewInverse));
	m_pTextureVariable->SetResource(m_pParticleTexture->GetResourceView());
	m_pWorldVariable->SetMatrix(reinterpret_cast<const float*>(&GetTransform()->GetWorldMatrix()));

	m_pGameContext->Engine->D3DeviceContext->IASetInputLayout(m_pInputLayout.Get());
	m_pGameContext->Engine->D3DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	UINT strides = sizeof(ParticleVertex);
	UINT offset = 0;
	m_pGameContext->Engine->D3DeviceContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &strides, &offset);

	m_pGameContext->Engine->D3DeviceContext->OMSetBlendState(m_pBlendState.Get(), nullptr, 0xFFFFFFFF);

	D3DX11_TECHNIQUE_DESC techDesc;
	m_pTechnique->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		m_pTechnique->GetPassByIndex(p)->Apply(0, m_pGameContext->Engine->D3DeviceContext);
		m_pGameContext->Engine->D3DeviceContext->Draw(m_ParticleCount, 0);
	}
}
