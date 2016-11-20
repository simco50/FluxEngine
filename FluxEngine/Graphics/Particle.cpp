//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

#include "Particle.h"
#include "../MathHelp.h"


Particle::Particle(ParticleEmitterSettings* emitterSettings) :
	m_pEmitterSettings(emitterSettings)
{
}


Particle::~Particle(void)
{
}

void Particle::Update()
{
	if (!m_IsActive)
		return;

	float deltaTime = GameTimer::DeltaTime();

	m_LifeTimer += deltaTime / m_pEmitterSettings->Lifetime;
	if (m_LifeTimer >= 1.0f)
	{
		m_IsActive = false;
		return;
	}

	m_VertexInfo.Position += m_pEmitterSettings->Velocity[m_LifeTimer] * deltaTime;
	Vector3 color = m_pEmitterSettings->Color[m_LifeTimer];
	m_VertexInfo.Color = Vector4(color.x, color.y, color.z, m_pEmitterSettings->Transparany[m_LifeTimer]);
	m_VertexInfo.Size = m_pEmitterSettings->Size[m_LifeTimer];
}

void Particle::Init(XMFLOAT3 initPosition)
{
	m_IsActive = true;

	//Energy Initialization
	m_LifeTimer = 0.0f;

	//Position initialization
	XMFLOAT3 randomDirection(1.0f, 0.0f, 0.0f);
	XMVECTOR xmDirection = XMLoadFloat3(&randomDirection);
	XMMATRIX randomMatrix = XMMatrixRotationRollPitchYaw(randF(-XM_PI, XM_PI), randF(-XM_PI, XM_PI), randF(-XM_PI, XM_PI));
	xmDirection = XMVector3Transform(xmDirection, randomMatrix);
	XMStoreFloat3(&randomDirection, xmDirection);

	float startingPointDistance = m_pEmitterSettings->EmitterRange;

	m_VertexInfo.Position = initPosition + startingPointDistance * randomDirection;

	//Size initialization
	m_VertexInfo.Size = m_pEmitterSettings->Size[0.0f];

	//Rotation Initialization
	m_VertexInfo.Rotation = randF(-XM_PI, XM_PI);
}