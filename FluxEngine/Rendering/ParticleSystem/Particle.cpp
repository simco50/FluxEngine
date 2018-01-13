//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "FluxEngine.h"

#include "Particle.h"

Particle::Particle(ParticleSystem* emitterSettings) :
	m_pEmitterSettings(emitterSettings)
{
}


Particle::~Particle()
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
	m_VertexInfo.Color = Vector4(color.x, color.y, color.z, m_pEmitterSettings->Transparancy[m_LifeTimer]);
	m_VertexInfo.Size = m_pEmitterSettings->Size[m_LifeTimer] * m_StartSize;
	m_VertexInfo.Rotation = m_StartRotation + m_pEmitterSettings->Rotation[m_LifeTimer];
}

void Particle::Init()
{
	m_IsActive = true;
	m_LifeTimer = 0.0f;
	GetPositionAndDirection(m_VertexInfo.Position, m_Direction);
	m_LifeTime = RandF(m_pEmitterSettings->Lifetime - m_pEmitterSettings->LifetimeVariance, m_pEmitterSettings->Lifetime + m_pEmitterSettings->LifetimeVariance);
	if(m_LifeTime < 0) m_LifeTime = 0;
	m_StartRotation = m_pEmitterSettings->RandomStartRotation ? RandF(0.0f, 360.0f) : 0.0f;
	m_StartVelocity = RandF(m_pEmitterSettings->StartVelocity - m_pEmitterSettings->StartVelocityVariance, m_pEmitterSettings->StartVelocity + m_pEmitterSettings->StartVelocityVariance);
	m_StartSize = RandF(m_pEmitterSettings->StartSize - m_pEmitterSettings->StartSizeVariance, m_pEmitterSettings->StartSize + m_pEmitterSettings->StartSizeVariance);
	if (m_StartSize < 0) m_StartSize = 0;
	Update();
}

void Particle::Reset()
{
	m_IsActive = false;
	m_LifeTimer = 0.0f;
}

void Particle::GetPositionAndDirection(Vector3& position, Vector3& direction)
{
	direction = Vector3(RandF(0, 1), 0, 0);
	if (m_pEmitterSettings->Shape.ShapeType == ParticleSystem::ShapeType::CIRCLE)
	{
		Matrix randomMatrix;
		randomMatrix = XMMatrixRotationRollPitchYaw(RandF(-XM_PI, XM_PI), RandF(-XM_PI, XM_PI), 0);
		direction = XMVector3Transform(direction, randomMatrix);

		if (m_pEmitterSettings->Shape.EmitFromShell)
			direction.Normalize();
		position = m_pEmitterSettings->Shape.Radius * direction;
		direction.Normalize();
		return;
	}
	if (m_pEmitterSettings->Shape.ShapeType == ParticleSystem::ShapeType::SPHERE)
	{
		Matrix randomMatrix;
		randomMatrix = XMMatrixRotationRollPitchYaw(RandF(-XM_PI, XM_PI), RandF(-XM_PI, XM_PI), RandF(-XM_PI, XM_PI));
		direction = XMVector3Transform(direction, randomMatrix);

		if (m_pEmitterSettings->Shape.EmitFromShell)
			direction.Normalize();
		position = m_pEmitterSettings->Shape.Radius * direction;
		return;
	}
	if (m_pEmitterSettings->Shape.ShapeType == ParticleSystem::ShapeType::CONE)
	{
		Matrix randomMatrix;
		randomMatrix = XMMatrixRotationRollPitchYaw(RandF(-XM_PI, XM_PI), RandF(-XM_PI, XM_PI), 0);
		position = XMVector3Transform(direction, randomMatrix);

		if (m_pEmitterSettings->Shape.EmitFromShell)
			position.Normalize();
		position *= m_pEmitterSettings->Shape.Radius;

		direction = Vector3();
		direction.y += RandF(0, m_pEmitterSettings->Lifetime);
		float offset = direction.y * tan(m_pEmitterSettings->Shape.Angle * XM_PI / 180.0f);
		direction.x += offset * position.x;
		direction.z += offset * position.z;

		if (m_pEmitterSettings->Shape.EmitFromVolume)
		{
			position += direction;
		}

		direction.Normalize();
		return;
	}
	if (m_pEmitterSettings->Shape.ShapeType == ParticleSystem::ShapeType::EDGE)
	{
		position = Vector3(RandF(-m_pEmitterSettings->Shape.Radius, m_pEmitterSettings->Shape.Radius), 0, 0);
		direction = Vector3(0, 0, 1);
	}
}
