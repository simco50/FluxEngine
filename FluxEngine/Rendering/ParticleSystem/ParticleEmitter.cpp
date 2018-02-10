//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "FluxEngine.h"

#include "ParticleEmitter.h"
#include "Particle.h"

#include "SceneGraph/Transform.h"
#include "Scenegraph/Scene.h"
#include "Scenegraph/SceneNode.h"

#include "Rendering/Geometry.h"
#include "Rendering/Material.h"
#include "Rendering/Core/Texture.h"
#include "Rendering/Camera/Camera.h"
#include "Rendering/Core/VertexBuffer.h"
#include "Rendering/Core/GraphicsDefines.h"
#include "Rendering/Core/Graphics.h"

ParticleEmitter::ParticleEmitter(Context* pContext, ParticleSystem* pSystem) :
	Drawable(pContext),
	m_pParticleSystem(pSystem)
{
	m_pGraphics = pContext->GetSubsystem<Graphics>();

	m_pGeometry = std::make_unique<Geometry>();
	m_Batches.resize(1);
	m_Batches[0].pGeometry = m_pGeometry.get();
	m_pMaterial = GetSubsystem<ResourceManager>()->Load<Material>("Resources/Materials/Particles.xml");
	m_pMaterial->SetDepthTestMode(CompareMode::ALWAYS);
	m_Batches[0].pMaterial = m_pMaterial;

	SetSystem(pSystem);
}

ParticleEmitter::~ParticleEmitter()
{
	FreeParticles();
}

void ParticleEmitter::SetSystem(ParticleSystem* pSettings)
{
	m_pParticleSystem = pSettings;
	if (pSettings == nullptr)
		return;

	if (m_pParticleSystem->ImagePath == "")
		m_pParticleSystem->ImagePath = ERROR_TEXTURE;

	FreeParticles();
	m_Particles.resize(m_pParticleSystem->MaxParticles);
	for (int i = 0; i < m_pParticleSystem->MaxParticles; i++)
		m_Particles[i] = new Particle(m_pParticleSystem);
	m_BufferSize = (int)m_Particles.size();
	CreateVertexBuffer(m_BufferSize);
	m_pGeometry->SetVertexBuffer(m_pVertexBuffer.get());

	m_pTexture = GetSubsystem<ResourceManager>()->Load<Texture>(pSettings->ImagePath);
	m_pMaterial->SetTexture(TextureSlot::Diffuse, m_pTexture);
	m_pMaterial->SetBlendMode(pSettings->BlendingMode);
	m_pMaterial->SetDepthEnabled(false);

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

void ParticleEmitter::OnSceneSet(Scene* pScene)
{
	Drawable::OnSceneSet(pScene);

	if (m_pParticleSystem)
	{
		m_Draw = true;
		CreateVertexBuffer(m_BufferSize);
		m_BurstIterator = m_pParticleSystem->Bursts.begin();
		if (m_pParticleSystem->PlayOnAwake)
			Play();
	}
	else
	{
		m_Draw = false;
		Stop();
	}
}

void ParticleEmitter::OnNodeSet(SceneNode* pNode)
{
	Drawable::OnNodeSet(pNode);
	m_Batches[0].pModelMatrix = &pNode->GetTransform()->GetWorldMatrix();
}

void ParticleEmitter::FreeParticles()
{
	for (size_t i = 0; i < m_Particles.size(); i++)
		delete m_Particles[i];
	m_Particles.clear();
}

void ParticleEmitter::CreateVertexBuffer(const int bufferSize)
{
	m_pVertexBuffer.reset();

	std::vector<VertexElement> elementDesc = {
		/*Position*/	VertexElement(VertexElementType::FLOAT3, VertexElementSemantic::POSITION, 0, false),
		/*Color*/		VertexElement(VertexElementType::FLOAT4, VertexElementSemantic::COLOR, 0, false),
		/*Scale*/		VertexElement(VertexElementType::FLOAT, VertexElementSemantic::TEXCOORD, 0, false),
		/*Rotation*/	VertexElement(VertexElementType::FLOAT, VertexElementSemantic::TEXCOORD, 1, false),
	};

	m_pVertexBuffer = std::make_unique<VertexBuffer>(m_pGraphics);
	m_pGeometry->SetVertexBuffer(m_pVertexBuffer.get());
	m_pVertexBuffer->Create(bufferSize, elementDesc, true);
}

void ParticleEmitter::SortParticles(const ParticleSortingMode sortMode)
{
	switch (sortMode)
	{
	case ParticleSortingMode::FrontToBack:
		std::sort(m_Particles.begin(), m_Particles.begin() + m_ParticleCount, [this](Particle* a, Particle* b)
		{
			float d1 = Vector3::DistanceSquared(a->GetVertexInfo().Position, m_pScene->GetCamera()->GetTransform()->GetWorldPosition());
			float d2 = Vector3::DistanceSquared(b->GetVertexInfo().Position, m_pScene->GetCamera()->GetTransform()->GetWorldPosition());
			return d1 > d2;
		});
		break;
	case ParticleSortingMode::BackToFront:
		std::sort(m_Particles.begin(), m_Particles.begin() + m_ParticleCount, [this](Particle* a, Particle* b)
		{
			float d1 = Vector3::DistanceSquared(a->GetVertexInfo().Position, m_pScene->GetCamera()->GetTransform()->GetWorldPosition());
			float d2 = Vector3::DistanceSquared(b->GetVertexInfo().Position, m_pScene->GetCamera()->GetTransform()->GetWorldPosition());
			return d1 < d2;
		});
	case ParticleSortingMode::OldestFirst:
		std::sort(m_Particles.begin(), m_Particles.begin() + m_ParticleCount, [this](Particle* a, Particle* b)
		{
			float lifeTimerA = a->GetLifeTimer();
			float lifeTimerB = b->GetLifeTimer();
			return lifeTimerA < lifeTimerB;
		});
		break;
	case ParticleSortingMode::YoungestFirst:
		std::sort(m_Particles.begin(), m_Particles.begin() + m_ParticleCount, [this](Particle* a, Particle* b)
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

void ParticleEmitter::CalculateBoundingBox()
{
	Vector3 max;
	auto p = std::max_element(m_Particles.begin(), m_Particles.end(), [this](const Particle* a, const Particle* b)
	{
		return a->GetVertexInfo().Position.x < b->GetVertexInfo().Position.x;
	});
	max.x = abs((*p)->GetVertexInfo().Position.x);
	p = std::max_element(m_Particles.begin(), m_Particles.end(), [this](const Particle* a, const Particle* b)
	{
		return a->GetVertexInfo().Position.y < b->GetVertexInfo().Position.y;
	});
	max.y = abs((*p)->GetVertexInfo().Position.y);
	p = std::max_element(m_Particles.begin(), m_Particles.end(), [this](const Particle* a, const Particle* b)
	{
		return a->GetVertexInfo().Position.z < b->GetVertexInfo().Position.z;
	});
	max.z = abs((*p)->GetVertexInfo().Position.z);
	m_BoundingBox = BoundingBox(Vector3(0.0f, max.y / 2.0f, 0.0f), Vector3(max.x, max.y / 2, max.z));
}

void ParticleEmitter::Update()
{
	if (m_Playing == false)
		return;

	CalculateBoundingBox();

	m_Timer += GameTimer::DeltaTime();
	if (m_Timer >= m_pParticleSystem->Duration && m_pParticleSystem->Loop)
	{
		m_Timer = 0;
		m_BurstIterator = m_pParticleSystem->Bursts.begin();
	}

	float emissionTime = 1.0f / m_pParticleSystem->Emission;
	m_ParticleSpawnTimer += GameTimer::DeltaTime();

	SortParticles(m_pParticleSystem->SortingMode);

	m_ParticleCount = 0;

	ParticleVertex* pBuffer = static_cast<ParticleVertex*>(m_pVertexBuffer->Map(true));

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
		CreateVertexBuffer(m_BufferSize);
	}
	m_pGeometry->SetDrawRange(PrimitiveType::POINTLIST, 0, m_ParticleCount);
}