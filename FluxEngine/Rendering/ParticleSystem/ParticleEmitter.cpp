//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

#include "ParticleEmitter.h"
#include "Content/ResourceManager.h"
#include "Particle.h"
#include "Core/Components/Transform.h"
#include "Rendering/Core/Texture.h"
#include "Rendering/Camera/Camera.h"
#include "Rendering/ParticleMaterial.h"
#include "../Core/VertexBuffer.h"
#include "../Core/GraphicsDefines.h"
#include "../Core/Graphics.h"

ParticleEmitter::ParticleEmitter(Graphics* pGraphics, ParticleSystem* pSystem) : 
	m_pParticleSystem(pSystem),
	m_pGraphics(pGraphics)
{
	m_Particles.resize(m_pParticleSystem->MaxParticles);
	for (int i = 0; i < m_pParticleSystem->MaxParticles; i++)
		m_Particles[i] = new Particle(m_pParticleSystem);
	m_BufferSize = m_pParticleSystem->MaxParticles;
}

ParticleEmitter::~ParticleEmitter()
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
	m_pMaterial = new ParticleMaterial(m_pGraphics);
	CreateVertexBuffer();
	m_pMaterial->SetTexture(m_pParticleSystem->ImagePath);
	m_pMaterial->SetBlendMode(m_pParticleSystem->BlendMode);
	m_BurstIterator = m_pParticleSystem->Bursts.begin();
	if (m_pParticleSystem->PlayOnAwake)
		Play();
}

void ParticleEmitter::CreateVertexBuffer()
{
	m_pVertexBuffer.reset();

	vector<VertexElement> elementDesc = {
		VertexElement(VertexElementType::VECTOR3, VertexElementSemantic::POSITION, 0, 0),
		VertexElement(VertexElementType::VECTOR4, VertexElementSemantic::COLOR, 0, 0),
		VertexElement(VertexElementType::FLOAT, VertexElementSemantic::TEXCOORD, 0, 0),
		VertexElement(VertexElementType::FLOAT, VertexElementSemantic::TEXCOORD, 1, 0),
	};
	
	m_pVertexBuffer = make_unique<VertexBuffer>(m_pGraphics);
	m_pVertexBuffer->Create(m_BufferSize, elementDesc, true);
}

void ParticleEmitter::SortParticles()
{
	switch (m_pParticleSystem->SortingMode)
	{
	case ParticleSortingMode::FrontToBack:
		sort(m_Particles.begin(), m_Particles.begin() + m_ParticleCount, [this](Particle* a, Particle* b)
		{
			float d1 = Vector3::DistanceSquared(a->GetVertexInfo().Position, m_pGameContext->Scene->Camera->GetTransform()->GetWorldPosition());
			float d2 = Vector3::DistanceSquared(b->GetVertexInfo().Position, m_pGameContext->Scene->Camera->GetTransform()->GetWorldPosition());
			return d1 > d2;
		});
		break;
	case ParticleSortingMode::BackToFront:
		sort(m_Particles.begin(), m_Particles.begin() + m_ParticleCount, [this](Particle* a, Particle* b)
		{
			float d1 = Vector3::DistanceSquared(a->GetVertexInfo().Position, m_pGameContext->Scene->Camera->GetTransform()->GetWorldPosition());
			float d2 = Vector3::DistanceSquared(b->GetVertexInfo().Position, m_pGameContext->Scene->Camera->GetTransform()->GetWorldPosition());
			return d1 < d2;
		});
	case ParticleSortingMode::OldestFirst:
		sort(m_Particles.begin(), m_Particles.begin() + m_ParticleCount, [](Particle* a, Particle* b)
		{
			float lifeTimerA = a->GetLifeTimer();
			float lifeTimerB = b->GetLifeTimer();
			return lifeTimerA < lifeTimerB;
		});
		break;
	case ParticleSortingMode::YoungestFirst:
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

	ParticleVertex* pBuffer = (ParticleVertex*)m_pVertexBuffer->Map(true);

	if(m_pParticleSystem->MaxParticles > (int)m_Particles.size())
	{
		int startIdx = (int)m_Particles.size();
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

	m_pVertexBuffer->Unmap();

	if (m_pParticleSystem->MaxParticles > m_BufferSize)
	{
		m_BufferSize = m_pParticleSystem->MaxParticles + 500;
		FLUX_LOG(WARNING, "ParticleEmitter::Render() > VertexBuffer too small! Increasing size...");
		CreateVertexBuffer();
	}

	/*
	m_pGraphics->SetVertexBuffer(m_pVertexBuffer.get());
	m_pGraphics->Draw(PrimitiveType::POINTLIST, 0, m_ParticleCount);
	*/
}