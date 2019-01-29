//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "FluxEngine.h"

#include "ParticleEmitter.h"
#include "Particle.h"

#include "Scenegraph/Scene.h"
#include "Scenegraph/SceneNode.h"

#include "Rendering/Geometry.h"
#include "Rendering/Material.h"
#include "Rendering/Camera/Camera.h"
#include "Rendering/Core/VertexBuffer.h"
#include "Rendering/Core/Graphics.h"
#include "Rendering/Core/Texture2D.h"

ParticleEmitter::ParticleEmitter(Context* pContext)
	: Drawable(pContext), m_pGeometry(std::make_unique<Geometry>())
{
	m_pGraphics = pContext->GetSubsystem<Graphics>();

	m_Batches.resize(1);
	m_Batches[0].pGeometry = m_pGeometry.get();
	m_pMaterial = GetSubsystem<ResourceManager>()->Load<Material>("Materials/Particles.xml");
	m_pMaterial->SetDepthTestMode(CompareMode::LESSEQUAL);
	m_pMaterial->SetDepthEnabled(true);
	m_pMaterial->SetDepthWrite(false);
	m_Batches[0].pMaterial = m_pMaterial;
}

ParticleEmitter::~ParticleEmitter()
{
	FreeParticles();
}

void ParticleEmitter::SetSystem(ParticleSystem* pSettings)
{
	AUTOPROFILE(ParticleEmitter_SetSystem);

	if (pSettings != m_pParticleSystem)
	{
		m_pParticleSystem = pSettings;
	}
	if (pSettings == nullptr)
	{
		return;
	}

	if (m_pParticleSystem->ImagePath.empty())
	{
		m_pParticleSystem->ImagePath = ERROR_TEXTURE;
	}

	FreeParticles();
	m_Particles.resize(m_pParticleSystem->MaxParticles);
	for (int i = 0; i < m_pParticleSystem->MaxParticles; i++)
	{
		m_Particles[i] = new Particle(m_pParticleSystem);
	}
	m_BufferSize = (int)m_Particles.size();
	CreateVertexBuffer(m_BufferSize);
	m_pGeometry->SetVertexBuffer(m_pVertexBuffer.get());

	m_pTexture = GetSubsystem<ResourceManager>()->Load<Texture2D>(pSettings->ImagePath);
	m_pMaterial->SetTexture(TextureSlot::Diffuse, m_pTexture);
	m_pMaterial->SetBlendMode(pSettings->BlendingMode);

	m_BurstIterator = m_pParticleSystem->Bursts.begin();
	Reset();
}

void ParticleEmitter::Reset()
{
	m_Timer = 0.0f;
	m_pParticleSystem->PlayOnAwake ? m_Playing = true : m_Playing = false;
	for (Particle* p : m_Particles)
	{
		p->Reset();
	}
}

void ParticleEmitter::OnSceneSet(Scene* pScene)
{
	Drawable::OnSceneSet(pScene);

	if (m_pParticleSystem)
	{
		CreateVertexBuffer(m_BufferSize);
		m_BurstIterator = m_pParticleSystem->Bursts.begin();
		if (m_pParticleSystem->PlayOnAwake)
			Play();
	}
	else
	{
		Stop();
	}
}

void ParticleEmitter::OnNodeSet(SceneNode* pNode)
{
	Drawable::OnNodeSet(pNode);
	m_Batches[0].pWorldMatrices = &pNode->GetWorldMatrix();
}

void ParticleEmitter::FreeParticles()
{
	for (Particle* pParticle : m_Particles)
	{
		delete pParticle;
	}
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
			float d1 = Vector3::DistanceSquared(a->GetVertexInfo().Position, m_pScene->GetCamera()->GetNode()->GetWorldPosition());
			float d2 = Vector3::DistanceSquared(b->GetVertexInfo().Position, m_pScene->GetCamera()->GetNode()->GetWorldPosition());
			return d1 > d2;
		});
		break;
	case ParticleSortingMode::BackToFront:
		std::sort(m_Particles.begin(), m_Particles.begin() + m_ParticleCount, [this](Particle* a, Particle* b)
		{
			float d1 = Vector3::DistanceSquared(a->GetVertexInfo().Position, m_pScene->GetCamera()->GetNode()->GetWorldPosition());
			float d2 = Vector3::DistanceSquared(b->GetVertexInfo().Position, m_pScene->GetCamera()->GetNode()->GetWorldPosition());
			return d1 < d2;
		});
	case ParticleSortingMode::OldestFirst:
		std::sort(m_Particles.begin(), m_Particles.begin() + m_ParticleCount, [](Particle* a, Particle* b)
		{
			float lifeTimerA = a->GetLifeTimer();
			float lifeTimerB = b->GetLifeTimer();
			return lifeTimerA < lifeTimerB;
		});
		break;
	case ParticleSortingMode::YoungestFirst:
		std::sort(m_Particles.begin(), m_Particles.begin() + m_ParticleCount, [](Particle* a, Particle* b)
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
	auto p = std::max_element(m_Particles.begin(), m_Particles.end(), [](const Particle* a, const Particle* b)
	{
		return a->GetVertexInfo().Position.x < b->GetVertexInfo().Position.x;
	});
	max.x = abs((*p)->GetVertexInfo().Position.x);
	p = std::max_element(m_Particles.begin(), m_Particles.end(), [](const Particle* a, const Particle* b)
	{
		return a->GetVertexInfo().Position.y < b->GetVertexInfo().Position.y;
	});
	max.y = abs((*p)->GetVertexInfo().Position.y);
	p = std::max_element(m_Particles.begin(), m_Particles.end(), [](const Particle* a, const Particle* b)
	{
		return a->GetVertexInfo().Position.z < b->GetVertexInfo().Position.z;
	});
	max.z = abs((*p)->GetVertexInfo().Position.z);
	m_BoundingBox = BoundingBox(Vector3(0.0f, max.y / 2.0f, 0.0f), Vector3(max.x, max.y / 2, max.z));
}

void ParticleEmitter::Update()
{
	if (m_Playing == false)
	{
		return;
	}

	AUTOPROFILE(ParticleEmitter_Update);
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
		{
			m_Particles[i] = new Particle(m_pParticleSystem);
		}
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
		else if(m_BurstIterator != m_pParticleSystem->Bursts.end() && m_Timer > m_BurstIterator->Time && burstParticles < m_BurstIterator->Value)
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
	{
		++m_BurstIterator;
	}

	m_pVertexBuffer->Unmap();

	if (m_pParticleSystem->MaxParticles > m_BufferSize)
	{
		m_BufferSize = m_pParticleSystem->MaxParticles + 500;
		FLUX_LOG(Warning, "ParticleEmitter::Render() > VertexBuffer too small! Increasing size...");
		CreateVertexBuffer(m_BufferSize);
	}
	m_pGeometry->SetDrawRange(PrimitiveType::POINTLIST, 0, m_ParticleCount);
}

void ParticleEmitter::CreateUI()
{
	if (m_pParticleSystem)
	{
		static const char* shapes[] =
		{
			"Circle",
			"Sphere",
			"Cone",
			"Edge",
		};

		ImGui::SliderFloat("Duration", &m_pParticleSystem->Duration, 0.0f, 20.0f);
		ImGui::Checkbox("Loop", &m_pParticleSystem->Loop);
		ImGui::SliderFloat("Lifetime", &m_pParticleSystem->Lifetime, 0.0f, 20.0f);
		ImGui::SliderFloat("Lifetime Variance", &m_pParticleSystem->LifetimeVariance, 0.0f, 10.0f);
		ImGui::SliderFloat("Start Velocity", &m_pParticleSystem->StartVelocity, 0.0f, 20.0f);
		ImGui::SliderFloat("Start Velocity Variance", &m_pParticleSystem->StartVelocityVariance, 0.0f, 10.0f);
		ImGui::SliderFloat("Start Size", &m_pParticleSystem->StartSize, 0.0f, 20.0f);
		ImGui::SliderFloat("Start Size Variance", &m_pParticleSystem->StartSizeVariance, 0.0f, 10.0f);
		ImGui::Checkbox("Random Start Rotation", &m_pParticleSystem->RandomStartRotation);
		ImGui::Checkbox("Loop", &m_pParticleSystem->Loop);
		ImGui::Checkbox("Play On Awake", &m_pParticleSystem->PlayOnAwake);
		ImGui::SliderInt("Max Particles", &m_pParticleSystem->MaxParticles, 0, 10000);
		ImGui::SliderInt("Emission", &m_pParticleSystem->Emission, 0, 1000);
		ImGui::Separator();
		ImGui::Combo("Shape", (int*)&m_pParticleSystem->Shape.ShapeType, [](void*, int index, const char** pText)
		{
			*pText = shapes[index];
			return true;
		}, nullptr, 4);
		ImGui::SliderFloat("Radius", &m_pParticleSystem->Shape.Radius, 0.0f, 20.0f);
		ImGui::Checkbox("Emit From Shell", &m_pParticleSystem->Shape.EmitFromShell);
		ImGui::Checkbox("Emit From Volume", &m_pParticleSystem->Shape.EmitFromVolume);
		ImGui::SliderFloat("Angle", &m_pParticleSystem->Shape.Angle, 0.0f, 180.0f);
	}
	else
	{
		ImGui::Text("No Particle System assigned");
	}

	Drawable::CreateUI();
}