//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

#include "ParticleEmitterComponent.h"
#include "../Content/ResourceManager.h"
#include "../Graphics/Particle.h"
#include "../Components/TransformComponent.h"
#include "../Debugging/Console.h"
#include "../Graphics/Texture.h"
#include "CameraComponent.h"

ParticleEmitterComponent::ParticleEmitterComponent(ParticleSystem* pSystem) : m_pParticleSystem(pSystem)
{
	m_Particles.resize(m_pParticleSystem->MaxParticles);
	for (int i = 0; i < m_pParticleSystem->MaxParticles; i++)
		m_Particles[i] = new Particle(m_pParticleSystem);
	m_BufferSize = m_pParticleSystem->MaxParticles;
}


ParticleEmitterComponent::~ParticleEmitterComponent(void)
{
	for (size_t i = 0; i < m_Particles.size(); i++)
		delete m_Particles[i];
	m_Particles.clear();
}

void ParticleEmitterComponent::SetSystem(ParticleSystem* pSettings)
{
	m_pParticleSystem = pSettings;
	if (m_pParticleSystem->ImagePath == "")
		m_pParticleSystem->ImagePath = ERROR_TEXTURE;
	CreateVertexBuffer();
	m_pParticleTexture = ResourceManager::Load<Texture>(m_pParticleSystem->ImagePath);
	m_BurstIterator = m_pParticleSystem->Bursts.begin();
	Reset();
}

void ParticleEmitterComponent::Reset()
{
	m_Timer = 0.0f;
	m_pParticleSystem->PlayOnAwake ? m_Playing = true : m_Playing = false;
	for (Particle* p : m_Particles)
		p->Reset();
}

void ParticleEmitterComponent::Initialize()
{
	if (m_pParticleSystem->ImagePath == "")
		m_pParticleSystem->ImagePath = ERROR_TEXTURE;
	LoadEffect();
	CreateVertexBuffer();
	m_pParticleTexture = ResourceManager::Load<Texture>(m_pParticleSystem->ImagePath);
	m_BurstIterator = m_pParticleSystem->Bursts.begin();
	if (m_pParticleSystem->PlayOnAwake)
		Play();
}

void ParticleEmitterComponent::LoadEffect()
{
	m_pEffect = ResourceManager::Load<ID3DX11Effect>("./Resources/Shaders/ParticleRenderer.fx");
	m_pTechnique = m_pEffect->GetTechniqueByIndex(0);
	m_pAlphaBlendingTechnique = m_pEffect->GetTechniqueByIndex(BlendMode::ALPHABLEND);
	m_pAdditiveBlendingTechnique = m_pEffect->GetTechniqueByIndex(BlendMode::ADDITIVE);

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

	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = m_BufferSize * sizeof(ParticleVertex);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bd.MiscFlags = 0;
	HRESULT hr = m_pGameContext->Engine->D3Device->CreateBuffer(&bd, nullptr, m_pVertexBuffer.GetAddressOf());
	Console::LogHRESULT("ParticleEmitterComponent::CreateVertexBuffer...", hr);
}

void ParticleEmitterComponent::SortParticles()
{
	switch (m_pParticleSystem->SortingMode)
	{
	case FrontToBack:
		sort(m_Particles.begin(), m_Particles.begin() + m_ParticleCount, [this](Particle* a, Particle* b)
		{
			float d1 = Vector3::DistanceSquared(a->GetVertexInfo().Position, m_pGameContext->Scene->Cameras[0]->GetTransform()->GetWorldPosition());
			float d2 = Vector3::DistanceSquared(b->GetVertexInfo().Position, m_pGameContext->Scene->Cameras[0]->GetTransform()->GetWorldPosition());
			return d1 > d2;
		});
		break;
	case BackToFront:
		sort(m_Particles.begin(), m_Particles.begin() + m_ParticleCount, [this](Particle* a, Particle* b)
		{
			float d1 = Vector3::DistanceSquared(a->GetVertexInfo().Position, m_pGameContext->Scene->Cameras[0]->GetTransform()->GetWorldPosition());
			float d2 = Vector3::DistanceSquared(b->GetVertexInfo().Position, m_pGameContext->Scene->Cameras[0]->GetTransform()->GetWorldPosition());
			return d1 < d2;
		});
	case OldestFirst:
		sort(m_Particles.begin(), m_Particles.begin() + m_ParticleCount, [](Particle* a, Particle* b)
		{
			float lifeTimerA = a->GetLifeTimer();
			float lifeTimerB = b->GetLifeTimer();
			return lifeTimerA < lifeTimerB;
		});
		break;
	case YoungestFirst: 
		sort(m_Particles.begin(), m_Particles.begin() + m_ParticleCount, [](Particle* a, Particle* b)
		{
			float lifeTimerA = a->GetLifeTimer();
			float lifeTimerB = b->GetLifeTimer();
			return lifeTimerA > lifeTimerB;
		});
		break;
	default: 
		break;
	}
}

void ParticleEmitterComponent::Update()
{
	if (m_Playing == false)
		return;

	m_Timer += GameTimer::DeltaTime();
	if (m_Timer >= m_pParticleSystem->Duration && m_pParticleSystem->Loop)
	{
		m_Timer = 0;
		m_BurstIterator = m_pParticleSystem->Bursts.begin();
	}

	float emissionTime = 1.0f / m_pParticleSystem->Emission;
	m_ParticleSpawnTimer += GameTimer::DeltaTime();

	SortParticles();

	m_ParticleCount = 0;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	m_pGameContext->Engine->D3DeviceContext->Map(m_pVertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	ParticleVertex* pBuffer = static_cast<ParticleVertex*>(mappedResource.pData);

	if(m_pParticleSystem->MaxParticles > (int)m_Particles.size())
	{
		int startIdx = m_Particles.size();
		m_Particles.resize(m_pParticleSystem->MaxParticles);
		for (int i = startIdx; i < m_pParticleSystem->MaxParticles; i++)
			m_Particles[i] = new Particle(m_pParticleSystem);
	}

	int burstParticles = 0;
	for (int i = 0; i < m_pParticleSystem->MaxParticles; i++)
	{
		Particle* p = m_Particles[i];
		//Update if active
		if (p->IsActive())
		{
			p->Update();
			pBuffer[m_ParticleCount] = p->GetVertexInfo();
			++m_ParticleCount;
		}
		//Spawn particle on burst tick
		else if(m_BurstIterator != m_pParticleSystem->Bursts.end() && m_Timer > m_BurstIterator->first && burstParticles < m_BurstIterator->second)
		{
			p->Init();
			pBuffer[m_ParticleCount] = p->GetVertexInfo();
			++m_ParticleCount;
			++burstParticles;
		}

		//Spawn particle on emission tick
		else if (m_ParticleSpawnTimer >= emissionTime && m_Timer < m_pParticleSystem->Duration)
		{
			p->Init();
			pBuffer[m_ParticleCount] = p->GetVertexInfo();
			++m_ParticleCount;
			m_ParticleSpawnTimer -= emissionTime;
		}
	}
	if (burstParticles > 0)
		++m_BurstIterator;

	m_pGameContext->Engine->D3DeviceContext->Unmap(m_pVertexBuffer.Get(), 0);
}

void ParticleEmitterComponent::Render()
{
	if (m_Playing == false)
		return;

	if(m_pParticleSystem->MaxParticles > m_BufferSize)
	{
		m_BufferSize = m_pParticleSystem->MaxParticles + 500;
		Console::Log("ParticleEmitterComponent::Render() > VertexBuffer too small! Increasing size...", LogType::WARNING);
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

	switch (m_pParticleSystem->BlendMode)
	{
	case ALPHABLEND: 
		m_pTechnique = m_pAlphaBlendingTechnique;
		break;
	case ADDITIVE: 
		m_pTechnique = m_pAdditiveBlendingTechnique;
		break;
	default: 
		break;
	}

	D3DX11_TECHNIQUE_DESC techDesc;
	m_pTechnique->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		m_pTechnique->GetPassByIndex(p)->Apply(0, m_pGameContext->Engine->D3DeviceContext);
		m_pGameContext->Engine->D3DeviceContext->Draw(m_ParticleCount, 0);
	}
}
