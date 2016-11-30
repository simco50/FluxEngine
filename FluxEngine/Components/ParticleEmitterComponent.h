#pragma once
#include "ComponentBase.h"
#include "../Graphics/ParticleSystem.h"

class Particle;
class Texture;

class ParticleEmitterComponent : public ComponentBase
{
public:
	ParticleEmitterComponent(ParticleSystem* pSystem = nullptr);
	~ParticleEmitterComponent(void);

	ParticleSystem* GetSettings() { return m_pParticleSystem; }

	void SetSystem(ParticleSystem* pSettings);

	void Reset() { m_Timer = 0.0f; }
	void Play() { m_Playing = true; }
	void Stop() { m_Playing = false; }
	void SetLoop(bool loop) { m_Loop = loop; }

protected:
	void Initialize();
	void Update();
	void Render();

private:
	void LoadEffect();
	void CreateVertexBuffer();
	void CreateBlendState();

	bool m_PlayOnAwake = true;
	bool m_Playing = false;
	bool m_Loop = true;
	float m_Duration = 3.0f;
	float m_Timer = 0.0f;

	vector<Particle*> m_Particles;
	ParticleSystem* m_pParticleSystem = nullptr;

	int m_ParticleCount = 0;
	int m_BufferSize = 0;
	float m_ParticleSpawnTimer = 0.0f;

	int m_ActiveParticles = 0;

	Unique_COM<ID3D11Buffer> m_pVertexBuffer;
	Unique_COM<ID3D11BlendState> m_pBlendState;

	ID3DX11Effect *m_pEffect = nullptr;
	ID3DX11EffectTechnique *m_pTechnique = nullptr;
	Unique_COM<ID3D11InputLayout> m_pInputLayout;
	ID3DX11EffectMatrixVariable *m_pViewProjectionVariable = nullptr;
	ID3DX11EffectMatrixVariable *m_pWorldVariable = nullptr;
	ID3DX11EffectMatrixVariable* m_pViewInverseVariable = nullptr;
	ID3DX11EffectShaderResourceVariable *m_pTextureVariable = nullptr;
	Texture *m_pParticleTexture = nullptr;
	wstring m_AssetFile;

private:
	// -------------------------
	// Disabling default copy constructor and default 
	// assignment operator.
	// -------------------------
	ParticleEmitterComponent(const ParticleEmitterComponent& yRef);
	ParticleEmitterComponent& operator=(const ParticleEmitterComponent& yRef);
};

