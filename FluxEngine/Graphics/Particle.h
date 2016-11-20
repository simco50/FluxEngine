#pragma once
#include "../Components/ParticleEmitterComponent.h"
struct GameContext;

class Particle
{
public:
	//Constructor, ParticleEmitterSettings (BY REFERENCE!)
	//the settings of the emitter, passing it by reference makes sure
	//that our particle has the latest settings of the emitter
	Particle(ParticleEmitterSettings* emitterSettings);
	~Particle(void);

	//Update method
	void Update();
	//Init, this method will 'reset' our particle
	void Init(XMFLOAT3 initPosition);

	//Method to retrieve the ParticleVertex information of our particle
	ParticleVertex GetVertexInfo() { return m_VertexInfo; }
	//Method to retrieve the state of our particle
	bool IsActive() { return m_IsActive; }

private:
	//ParticleVertex information, used by the emitter's vertexbuffer
	ParticleVertex m_VertexInfo = ParticleVertex();
	//The emitter's settings (by reference)
	ParticleEmitterSettings* m_pEmitterSettings;
	//An active particle is still alive,
	//An inactive particle can be reused (ReInitialized)
	bool m_IsActive = false;
	//Energy passed since the particle became active
	float m_LifeTimer = 0.0f;
	//Initial size of the particle
	float m_InitSize = 0.0f;

	// -------------------------
	// Disabling default copy constructor and default 
	// assignment operator.
	// -------------------------
	Particle(const Particle& yRef);
	Particle& operator=(const Particle& yRef);
};

