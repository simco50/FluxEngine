//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

#include "ParticleEmitter.h"
#include "Content/ResourceManager.h"
#include "Particle.h"
#include "Core/Components/Transform.h"
#include "Debugging/Console.h"
#include "Rendering/Core/Texture.h"
#include "Rendering/Camera/Camera.h"
#include "Materials/ParticleMaterial.h"

ParticleEmitter::ParticleEmitter(ParticleSystem* pSystem) : m_pParticleSystem(pSystem)
{
	m_Particles.resize(m_pParticleSystem->MaxParticles);
	for (int i = 0; i < m_pParticleSystem->MaxParticles; i++)
		m_Particles[i] = new Particle(m_pParticleSystem);
	m_BufferSize = m_pParticleSystem->MaxParticles;
}


ParticleEmitter::~ParticleEmitter(void)
{
	for (size_t i = 0; i < m_Particles.size(); i++)
		delete m_Particles[i];
	m_Particles.clear();

	delete m_pMaterial;
	m_pMaterial = nullptr;
}

void ParticleEmitter::SetSystem(ParticleSystem* pSettings)
{
	m_pParticleSystem = pSettings;
	if (m_pParticleSystem->ImagePath == "")
		m_pParticleSystem->ImagePath = ERROR_TEXTURE;
	CreateVertexBuffer();
	m_pMaterial->SetTexture(m_pParticleSystem->ImagePath);
	m_pMaterial->SetBlendMode(m_pParticleSystem->BlendMode);
	m_BurstIterator = m_pParticleSystem->Bursts.begin();
	Reset();
}

void ParticleEmitter::Reset()
{
	m_Timer = 0.0f;
	m_pParticleSystem->PlayOnAwake ? m_Playing = true : m_Playing = false;
	for (Particle* p : m_Particles)
		p->Reset();
}

void ParticleEmitter::Initialize()
{
	if (m_pParticleSystem->ImagePath == "")
		m_pParticleSystem->ImagePath = ERROR_TEXTURE;
	m_pMaterial = new ParticleMaterial();
	m_pMaterial->Initialize(m_pGameContext);
	CreateVertexBuffer();
	m_pMaterial->SetTexture(m_pParticleSystem->ImagePath);
	m_pMaterial->SetBlendMode(m_pParticleSystem->BlendMode);
	m_BurstIterator = m_pParticleSystem->Bursts.begin();
	if (m_pParticleSystem->PlayOnAwake)
		Play();
}

void ParticleEmitter::CreateVertexBuffer()
{
	m_pVertexBuffer.Reset();

	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = m_BufferSize * sizeof(ParticleVertex);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bd.MiscFlags = 0;
	HR(RenderSystem::Instance().GetDevice()->CreateBuffer(&bd, nullptr, m_pVertexBuffer.GetAddressOf()));
}

void ParticleEmitter::SortParticles()
{
	switch (m_pParticleSystem->SortingMode)
	{
	case FrontToBack:
		sort(m_Particles.begin(), m_Particles.begin() + m_ParticleCount, [this](Particle* a, Particle* b)
		{
			float d1 = Vector3::DistanceSquared(a->GetVertexInfo().Position, m_pGameContext->Scene->Camera->GetTransform()->GetWorldPosition());
			float d2 = Vector3::DistanceSquared(b->GetVertexInfo().Position, m_pGameContext->Scene->Camera->GetTransform()->GetWorldPosition());
			return d1 > d2;
		});
		break;
	case BackToFront:
		sort(m_Particles.begin(), m_Particles.begin() + m_ParticleCount, [this](Particle* a, Particle* b)
		{
			float d1 = Vector3::DistanceSquared(a->GetVertexInfo().Position, m_pGameContext->Scene->Camera->GetTransform()->GetWorldPosition());
			float d2 = Vector3::DistanceSquared(b->GetVertexInfo().Position, m_pGameContext->Scene->Camera->GetTransform()->GetWorldPosition());
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

void ParticleEmitter::Update()
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
	RenderSystem::Instance().GetDeviceContext()->Map(m_pVertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
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

	RenderSystem::Instance().GetDeviceContext()->Unmap(m_pVertexBuffer.Get(), 0);

	if (m_pParticleSystem->MaxParticles > m_BufferSize)
	{
		m_BufferSize = m_pParticleSystem->MaxParticles + 500;
		FLUX_LOG(WARNING, "ParticleEmitter::Render() > VertexBuffer too small! Increasing size...");
		CreateVertexBuffer();
	}

	RenderItem item;
	item.Material = m_pMaterial;
	item.Topology = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
	item.VertexBuffers = { m_pVertexBuffer.Get() };
	item.VertexCount = m_ParticleCount;
	item.WorldMatrix = GetTransform()->GetWorldMatrix();

	RenderSystem::Instance().Submit(item);
}