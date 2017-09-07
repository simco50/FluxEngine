#pragma once
#include "ComponentBase.h"
#include "../Graphics/ParticleSystem.h"

class Particle;
class Texture;

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
	void Render();

private:
	void LoadEffect();
	void CreateVertexBuffer();
	void CreateBlendState();
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

	ID3DX11Effect *m_pEffect = nullptr;
	ID3DX11EffectTechnique *m_pAlphaBlendingTechnique = nullptr;
	ID3DX11EffectTechnique *m_pAdditiveBlendingTechnique = nullptr;
	ID3DX11EffectTechnique *m_pTechnique = nullptr;

	Unique_COM<ID3D11InputLayout> m_pInputLayout;
	ID3DX11EffectMatrixVariable *m_pViewProjectionVariable = nullptr;
	ID3DX11EffectMatrixVariable *m_pWorldVariable = nullptr;
	ID3DX11EffectMatrixVariable* m_pViewInverseVariable = nullptr;
	ID3DX11EffectShaderResourceVariable *m_pTextureVariable = nullptr;
	Texture *m_pParticleTexture = nullptr;
	wstring m_AssetFile;
};

