#pragma once
#include "ParticleSystem.h"

class Particle
{
public:
	explicit Particle(ParticleSystem* emitterSettings);
	~Particle();

	void Update();
	void Init();

	const ParticleVertex &GetVertexInfo() const { return m_VertexInfo; }
	bool IsActive() const { return m_IsActive; }
	void Reset();

	float GetLifeTimer() const { return m_LifeTimer; }

private:
	void GetPositionAndDirection(Vector3& position, Vector3& direction) const;
	ParticleVertex m_VertexInfo = ParticleVertex();
	ParticleSystem* m_pEmitterSettings;
	float m_LifeTimer = 0.0f;
	Vector3 m_Direction;
	float m_StartRotation = 0.0f;

	float m_LifeTime = 0;
	float m_StartVelocity = 0;
	float m_StartSize = 0;
	bool m_IsActive = false;
};