#pragma once
#include "../Components/ParticleEmitterComponent.h"
struct GameContext;

class Particle
{
public:
	Particle(ParticleEmitterSettings* emitterSettings);
	~Particle(void);

	void Update();
	void Init();

	const ParticleVertex &GetVertexInfo() { return m_VertexInfo; }
	bool IsActive() { return m_IsActive; }
	void Reset();

private:
	void GetPositionAndDirection(Vector3& position, Vector3& direction);
	ParticleVertex m_VertexInfo = ParticleVertex();
	ParticleEmitterSettings* m_pEmitterSettings;
	bool m_IsActive = false;
	float m_LifeTimer = 0.0f;
	float m_InitSize = 0.0f;
	Vector3 m_Direction;
	float m_StartRotation = 0.0f;

	float m_LifeTime = 0;
	float m_StartVelocity = 0;
};

