#pragma once
#include "ParticleSystem.h"

struct GameContext;


class Particle
{
public:
	Particle(ParticleSystem* emitterSettings);
	~Particle(void);

	void Update();
	void Init();

	const ParticleVertex &GetVertexInfo() { return m_VertexInfo; }
	bool IsActive() { return m_IsActive; }
	void Reset();

	float GetLifeTimer() const { return m_LifeTimer; }

private:
	void GetPositionAndDirection(Vector3& position, Vector3& direction);
	ParticleVertex m_VertexInfo = ParticleVertex();
	ParticleSystem* m_pEmitterSettings;
	bool m_IsActive = false;
	float m_LifeTimer = 0.0f;
	float m_InitSize = 0.0f;
	Vector3 m_Direction;
	float m_StartRotation = 0.0f;

	float m_LifeTime = 0;
	float m_StartVelocity = 0;
	float m_StartSize = 0;
};

