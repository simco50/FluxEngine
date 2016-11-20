//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

#include "Particle.h"
#include "../Helpers/MathHelp.h"
#include "../Helpers/BinaryReader.h"

Particle::Particle(ParticleEmitterSettings* emitterSettings) :
	m_pEmitterSettings(emitterSettings)
{
}


Particle::~Particle(void)
{
}

void Particle::Update()
{
	float deltaTime = GameTimer::DeltaTime();

	m_LifeTimer += deltaTime / m_LifeTime;
	if (m_LifeTimer >= 1.0f)
	{
		m_IsActive = false;
		return;
	}

	Vector3 movement = Vector3();
	//Constant velocity
	movement += m_Direction * m_StartVelocity;
	//World space velocity
	movement += m_pEmitterSettings->Velocity[m_LifeTimer];
	//Local space velocity
	Vector3 localVel = m_pEmitterSettings->LocalVelocity[m_LifeTimer];
	if (localVel.LengthSquared() > 0)
	{
		Vector3 up = Vector3(0, 1, 0);
		Vector3 dir = m_Direction;
		Vector3 right = dir.Cross(up);
		up = right.Cross(dir);
		Matrix mat = Matrix(right, up, dir);
		movement += Vector3::Transform(localVel, mat);
	}
	m_VertexInfo.Position += movement * deltaTime;

	Vector3 color = m_pEmitterSettings->Color[m_LifeTimer];
	m_VertexInfo.Color = Vector4(color.x, color.y, color.z, m_pEmitterSettings->Transparany[m_LifeTimer]);
	m_VertexInfo.Size = m_pEmitterSettings->Size[m_LifeTimer];

	m_VertexInfo.Rotation = m_StartRotation + m_pEmitterSettings->Rotation[m_LifeTimer];
}

void Particle::Init()
{
	m_IsActive = true;
	m_LifeTimer = 0.0f;
	GetPositionAndDirection(m_VertexInfo.Position, m_Direction);
	m_StartRotation = m_pEmitterSettings->RandomStartRotation ? randF(0.0f, XM_2PI) : 0.0f;
	m_LifeTime = randF(m_pEmitterSettings->Lifetime - m_pEmitterSettings->LifetimeVariance, m_pEmitterSettings->Lifetime + m_pEmitterSettings->LifetimeVariance);
	m_StartVelocity = randF(m_pEmitterSettings->StartVelocity - m_pEmitterSettings->StartVelocityVariance, m_pEmitterSettings->StartVelocity + m_pEmitterSettings->StartVelocityVariance);
	
	Update();
}

void Particle::Reset()
{
	m_IsActive = false;
	m_LifeTimer = 0.0f;
}

void Particle::GetPositionAndDirection(Vector3& position, Vector3& direction)
{
	direction = Vector3(randF(0, 1), 0, 0);
	if (m_pEmitterSettings->Shape.ShapeType == ParticleEmitterSettings::ShapeType::CIRCLE)
	{
		Matrix randomMatrix;
		randomMatrix = XMMatrixRotationRollPitchYaw(randF(-XM_PI, XM_PI), randF(-XM_PI, XM_PI), 0);
		direction = XMVector3Transform(direction, randomMatrix);

		if (m_pEmitterSettings->Shape.EmitFromShell)
			direction.Normalize();
		position = m_pEmitterSettings->Shape.Radius * direction;
		return;
	}
	if (m_pEmitterSettings->Shape.ShapeType == ParticleEmitterSettings::ShapeType::SPHERE)
	{
		Matrix randomMatrix;
		randomMatrix = XMMatrixRotationRollPitchYaw(randF(-XM_PI, XM_PI), randF(-XM_PI, XM_PI), randF(-XM_PI, XM_PI));
		direction = XMVector3Transform(direction, randomMatrix);

		if (m_pEmitterSettings->Shape.EmitFromShell)
			direction.Normalize();
		position = m_pEmitterSettings->Shape.Radius * direction;
		return;
	}
	if (m_pEmitterSettings->Shape.ShapeType == ParticleEmitterSettings::ShapeType::CONE)
	{
		Matrix randomMatrix;
		randomMatrix = XMMatrixRotationRollPitchYaw(randF(-XM_PI, XM_PI), randF(-XM_PI, XM_PI), 0);
		direction = XMVector3Transform(direction, randomMatrix);

		if (m_pEmitterSettings->Shape.EmitFromShell)
			direction.Normalize();

		Vector3 originPosition = direction;
		position = originPosition;
		position.y += randF(0, m_pEmitterSettings->Lifetime);
		float offset = position.y * tan(m_pEmitterSettings->Shape.Angle * XM_PI / 180.0f);
		position.x += offset * direction.x;
		position.z += offset * direction.z;
		direction = position - originPosition;

		if (!m_pEmitterSettings->Shape.EmitFromVolume)
			position = originPosition;

		direction.Normalize();
		return;
	}
	if (m_pEmitterSettings->Shape.ShapeType == ParticleEmitterSettings::ShapeType::EDGE)
	{
		position = Vector3(randF(-m_pEmitterSettings->Shape.Radius, m_pEmitterSettings->Shape.Radius), 0, 0);
		direction = Vector3(0, 0, 1);
	}
}
