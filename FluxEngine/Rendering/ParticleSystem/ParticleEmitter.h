#pragma once
#include "Core/Components/ComponentBase.h"

class Particle;
class Texture;
class ParticleMaterial;
struct ParticleSystem;

class ParticleEmitter : public ComponentBase
{
public:
	ParticleEmitter(ParticleSystem* pSystem = nullptr);
	~ParticleEmitter(void);
	ParticleEmitter(const ParticleEmitter& other) = delete;
	ParticleEmitter operator=(const ParticleEmitter& other) = delete;

	ParticleSystem* GetSettings() const { return m_pParticleSystem; }

	void SetSystem(ParticleSystem* pSettings);

	void Reset();

	void Play() { m_Playing = true; }
	void Stop() { m_Playing = false; }

	int GetParticleCount() const { return m_ParticleCount; }

protected:
	void Initialize();
	void Update();

private:
	void CreateVertexBuffer();
	void SortParticles();

	bool m_Playing = false;
	float m_Timer = 0.0f;

	map<float, int>::iterator m_BurstIterator;

	vector<Particle*> m_Particles;
	ParticleSystem* m_pParticleSystem = nullptr;

	int m_ParticleCount = 0;
	int m_BufferSize = 0;
	float m_ParticleSpawnTimer = 0.0f;

	Unique_COM<ID3D11Buffer> m_pVertexBuffer;

	ParticleMaterial* m_pMaterial = nullptr;
	wstring m_AssetFile;
};

