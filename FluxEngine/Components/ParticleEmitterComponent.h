#pragma once
#include "ComponentBase.h"
#include "../Helpers/KeyframeValue.h"

struct ParticleEmitterSettings
{
	ParticleEmitterSettings() :
		Lifetime(1.0f),
		LifetimeVariance(0.0f),
		StartVelocity(0.0f),
		StartVelocityVariance(0.0f),
		Size(1.0f),
		Velocity(Vector3()),
		LocalVelocity(Vector3()),
		Color(Vector3(1, 1, 1)),
		Transparany(1),
		Rotation(0.0f)
	{}

	float Lifetime;
	float LifetimeVariance;

	float StartVelocity;
	float StartVelocityVariance;

	KeyframeValue<float> Size;
	KeyframeValue<Vector3> Velocity;
	KeyframeValue<Vector3> LocalVelocity;
	KeyframeValue<Vector3> Color;
	KeyframeValue<float> Transparany;
	KeyframeValue<float> Rotation;
	bool RandomStartRotation = true;

	enum class ShapeType
	{
		CIRCLE,
		SPHERE,
		CONE,
		EDGE,
	};
	struct Shape
	{
		ShapeType ShapeType = ShapeType::SPHERE;
		float Radius = 1.0f;
		bool EmitFromShell = false;
		bool EmitFromVolume = false;
		float Angle = 30.0f;
	};
	Shape Shape;
};

struct ParticleVertex
{
	ParticleVertex(Vector3 pos = Vector3(), Vector4 col = (Vector4)Colors::White, float size = 5.0f, float rotation = 0) :
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

enum ParticleSortingMode
{
	OldestFirst,
	YoungestFirst,
	ByDistance,
};

class ParticleEmitterComponent : public ComponentBase
{
public:
	ParticleEmitterComponent(const wstring& assetFile, int emission, int maxParticles, bool playOnAwake = true);
	~ParticleEmitterComponent(void);

	ParticleEmitterSettings* GetSettings() { return &m_Settings; }
	void SetEmission(const int emission) { m_Emission = emission; }
	void SetMaxParticles(const int maxParticles);
	void Reset() { m_Timer = 0.0f; }
	void Play() { m_Playing = true; }
	void Stop() { m_Playing = false; }
	void SetLoop(bool loop) { m_Loop = loop; }
	void SetSortingMode(ParticleSortingMode mode) { m_SortingMode = mode; }

protected:
	void Initialize();
	void Update();
	void Render();

private:
	void LoadEffect();
	void CreateVertexBuffer();
	void CreateBlendState();

	bool m_PlayOnAwake;
	bool m_Playing = false;
	bool m_Loop = true;
	float m_Duration = 3.0f;
	float m_Timer = 0.0f;

	vector<Particle*> m_Particles;
	ParticleEmitterSettings m_Settings = ParticleEmitterSettings();
	int m_ParticleCount = 0;
	int m_MaxParticles;
	int m_BufferSize = 0;
	float m_ParticleSpawnTimer = 0.0f;
	int m_Emission;
	int m_ActiveParticles = 0;
	ParticleSortingMode m_SortingMode = ByDistance;

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

