#pragma once
#include "Rendering/Drawable.h"

class Particle;
class Texture2D;
class Graphics;
class VertexBuffer;
class ParticleSystem;
class Material;

enum class ParticleSortingMode;

class ParticleEmitter : public Drawable
{
	FLUX_OBJECT(ParticleEmitter, Drawable)

public:
	ParticleEmitter(Context* pContext);
	~ParticleEmitter();
	ParticleEmitter(const ParticleEmitter& other) = delete;
	ParticleEmitter operator=(const ParticleEmitter& other) = delete;

	ParticleSystem* GetSettings() const { return m_pParticleSystem; }

	void SetSystem(ParticleSystem* pSettings);

	void Reset();

	void Play() { m_Playing = true; }
	void Stop() { m_Playing = false; }

	int GetParticleCount() const { return m_ParticleCount; }

	virtual void CreateUI() override;

protected:
	virtual void OnSceneSet(Scene* pScene) override;
	virtual void OnNodeSet(SceneNode* pNode) override;
	virtual void Update() override;

private:
	void FreeParticles();
	void CreateVertexBuffer(int bufferSize);
	void SortParticles(ParticleSortingMode sortMode);

	void CalculateBoundingBox();

	bool m_Playing = false;
	float m_Timer = 0.0f;

	std::map<float, int>::iterator m_BurstIterator;

	std::vector<Particle*> m_Particles;
	ParticleSystem* m_pParticleSystem = nullptr;

	int m_ParticleCount = 0;
	int m_BufferSize = 0;
	float m_ParticleSpawnTimer = 0.0f;

	std::unique_ptr<Geometry> m_pGeometry;
	std::unique_ptr<VertexBuffer> m_pVertexBuffer;

	Material* m_pMaterial = nullptr;
	Texture2D* m_pTexture = nullptr;

	std::wstring m_AssetFile;

	Graphics* m_pGraphics;
};

