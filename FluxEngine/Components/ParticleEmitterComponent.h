#pragma once
#include "ComponentBase.h"
#include "../Helpers/KeyframeValue.h"

struct ParticleEmitterSettings
{
	ParticleEmitterSettings() :
		Size(1.0f),
		Lifetime(1.0f),
		Velocity(Vector3()),
		EmitterRange(10.0f),
		Color((Vector3)Colors::White),
		Transparany(1)
	{}
	float Lifetime;
	float EmitterRange;
	KeyframeValue<float> Size;
	KeyframeValue<Vector3> Velocity;
	KeyframeValue<Vector3> Color;
	KeyframeValue<float> Transparany;
};

struct ParticleVertex
{
public:

	ParticleVertex(Vector3 pos = Vector3(0, 0, 0), Vector4 col = (Vector4)Colors::White, float size = 5.0f, float rotation = 0) :
		Position(pos),
		Color(col),
		Size(size),
		Rotation(rotation) {}

	Vector3 Position;
	Vector4 Color;
	float Size;
	float Rotation;
};

class Particle;
class Texture;

class ParticleEmitterComponent : public ComponentBase
{
public:
	//Constructor, particle texture path and max particles
	ParticleEmitterComponent(const wstring& assetFile, int particleCount = 50);
	~ParticleEmitterComponent(void);

	ParticleEmitterSettings* GetSettings() { return &m_Settings; }

protected:
	void Initialize();
	void Update();
	void Render();

private:
	bool m_Loop = true;
	float m_Duration = 1.0f;
	float m_Timer = 0.0f;

	//Method to load effect-related stuff
	void LoadEffect();
	//Method to create the vertex buffer
	void CreateVertexBuffer();

	//The actual vertex buffer, containing ParticleVertex information for each Particle
	ID3D11Buffer* m_pVertexBuffer = nullptr;

	//The effect we'll use to render the particles
	ID3DX11Effect *m_pEffect = nullptr;
	//The default technique
	ID3DX11EffectTechnique *m_pDefaultTechnique = nullptr;
	//Shader matrix variables
	ID3DX11EffectMatrixVariable *m_pWvpVariable = nullptr, *m_pViewInverseVariable = nullptr;
	//Shader texture variable
	ID3DX11EffectShaderResourceVariable *m_pTextureVariable = nullptr;
	//ShaderResourceView, containing the particle texture
	Texture *m_pParticleTexture = nullptr;

	//Vertex input layout variables
	ID3D11InputLayout *m_pInputLayout = nullptr;
	UINT m_pInputLayoutSize = 0;

	//Vector of particle pointers
	vector<Particle*> m_Particles;
	//The settings for this particle system
	ParticleEmitterSettings m_Settings = ParticleEmitterSettings();
	//The total amount of particles (m_Particles.size() == m_ParticleCount)
	int m_ParticleCount = 0;
	//The active particles for a certain frame
	int m_ActiveParticles = 0;
	//Total seconds since the last particle initialisation
	float m_LastParticleInit = 0.0f;
	//string containing the path to the particle texture
	wstring m_AssetFile;

private:
	// -------------------------
	// Disabling default copy constructor and default 
	// assignment operator.
	// -------------------------
	ParticleEmitterComponent(const ParticleEmitterComponent& yRef);
	ParticleEmitterComponent& operator=(const ParticleEmitterComponent& yRef);
};

